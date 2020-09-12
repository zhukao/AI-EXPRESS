package main

import (
	"archive/tar"
	"bufio"
	"bytes"
	"compress/gzip"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

// 解析日志文件
func parseLog(file string, lincb func(string) error) error {
	logf, err := os.Open(os.Args[1])
	if err != nil {
		return err
	}
	defer logf.Close()

	// 读取每一行
	recordBuf := bufio.NewReader(logf)
	for {
		line, _, err := recordBuf.ReadLine()
		if err == io.EOF {
			break
		} else if err != nil {
			return err
		}
		err = lincb(string(line))
		if err != nil {
			return err
		}
	}
	return nil
}

func generateJSON(file string) string {
	var threadProf = make(map[string]*Profile)
	// 创建flame对象
	flame := CreateFlameGraph()
	flame.SetFlameName(filepath.Base(file))

	// 打开文件
	err := parseLog(file, func(line string) error {
		items := strings.SplitN(line, "|", 5)
		if len(items) < 5 {
			return fmt.Errorf("Record format invailed! Items:%d", len(items))
		}

		// 判断是何种类型
		if items[0] == "S" {
			at, err := strconv.ParseFloat(items[3], 64)
			if err != nil {
				return err
			}

			prof, exist := threadProf[items[1]]
			if !exist {
				prof = flame.CreateProfile(ProfileTypeEvented, items[1], ProfileUnitSecond)
				threadProf[items[1]] = prof
			}

			prof.OpenFrame(Frame{
				Name: items[2],
			}, at)
		} else if items[0] == "E" {
			at, err := strconv.ParseFloat(items[3], 64)
			if err != nil {
				return err
			}

			prof, exist := threadProf[items[1]]
			if !exist {
				prof = flame.CreateProfile(ProfileTypeEvented, items[1], ProfileUnitSecond)
				threadProf[items[1]] = prof
			}

			prof.CloseFrame(items[2], at)
		} else {
			return fmt.Errorf("Record format invailed! Items[0]:%s", items[0])
		}
		return nil
	})

	if err != nil {
		panic(err)
	}

	profJSON, err := flame.Marshal()
	if err != nil {
		panic(err)
	}

	return profJSON
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Usage: %s <log file>\n", os.Args[0])
		os.Exit(1)
	}

	// 创建临时目录
	dir, err := ioutil.TempDir("", "xstreamprof_")
	if err != nil {
		panic(err)
	}
	defer os.RemoveAll(dir)

	// 解压相关文件
	bufferReader := bytes.NewReader(verndorData[:])
	fileReader, err := gzip.NewReader(bufferReader)
	if err != nil {
		panic(err)
	}
	defer fileReader.Close()

	tarBallReader := tar.NewReader(fileReader)

	for {
		header, err := tarBallReader.Next()
		if err != nil {
			if err == io.EOF {
				break
			}
			panic(err)
		}

		filename := filepath.Join(dir, header.Name)

		switch header.Typeflag {
		case tar.TypeDir:
			err = os.MkdirAll(filename, os.FileMode(header.Mode))
			if err != nil {
				panic(err)
			}

		case tar.TypeReg:
			writer, err := os.Create(filename)
			if err != nil {
				panic(err)
			}

			io.Copy(writer, tarBallReader)
			writer.Close()

			err = os.Chmod(filename, os.FileMode(header.Mode))
			if err != nil {
				panic(err)
			}

		default:
			fmt.Printf("Unable to untar type : %c in file %s", header.Typeflag, filename)
		}
	}

	// 启动服务
	http.HandleFunc("/data", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, generateJSON(os.Args[1]))
	})
	http.Handle("/", http.FileServer(http.Dir(filepath.Join(dir, "speedscope"))))
	fmt.Println("Visit: http://127.0.0.1:8300/#profileURL=http://127.0.0.1:8300/data")
	err = http.ListenAndServe(":8300", nil)
	if err != nil {
		panic(err)
	}
	os.Exit(0)
}
