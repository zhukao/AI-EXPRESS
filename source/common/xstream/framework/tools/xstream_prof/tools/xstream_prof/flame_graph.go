package main

import (
	"encoding/json"
	"errors"
)

// Frame 函数调用栈帧
type Frame struct {
	Name string `json:"name"`           // 函数名
	File string `json:"file,omitempty"` // 所在文件
	Line uint   `json:"line,omitempty"` // 所在的行
	Col  uint   `json:"col,omitempty"`  // 所在的列
}

// Event类型的Profile
const (
	ProfileTypeEvented = "evented"

	ProfileUnitNone        = "none"
	ProfileUnitNanoSecond  = "nanoseconds"
	ProfileUnitMicroSecond = "microseconds"
	ProfileUnitMilliSecond = "milliseconds"
	ProfileUnitSecond      = "seconds"
	ProfileUnitByte        = "bytes"
)

// 共享的数据
type shared struct {
	Frames []Frame `json:"frames"`
}

type event struct {
	Type  string  `json:"type"`
	Frame uint    `json:"frame"`
	At    float64 `json:"at"`
}

// Profile 剖析记录,目前只支持 evented
type Profile struct {
	Type       string  `json:"type"`
	Name       string  `json:"name"`
	Unit       string  `json:"unit"`
	StartValue float64 `json:"startValue"`
	EndValue   float64 `json:"endValue"`
	Events     []event `json:"events"`
	parent     *FlameGraph
}

// FlameJSON FlameGraph描述结构
type flameJSON struct {
	Exporter           string     `json:"exporter,omitempty"`
	Name               string     `json:"name,omitempty"`
	ActiveProfileIndex int        `json:"activeProfileIndex,omitempty"`
	Schema             string     `json:"$schema"`
	Shared             shared     `json:"shared"`
	Profiles           []*Profile `json:"profiles"`
}

// FlameGraph 火焰图
type FlameGraph struct {
	profileRecord flameJSON       // 剖析记录
	frameIndex    map[string]uint // 栈帧编号索引
}

// CreateFlameGraph 创建一个新的FlameGraph
func CreateFlameGraph() *FlameGraph {
	obj := &FlameGraph{
		profileRecord: flameJSON{
			Exporter: "XStreamProf@0.0.1",
			Schema:   "https://www.speedscope.app/file-format-schema.json",
			Shared: shared{
				Frames: make([]Frame, 0),
			},
			Profiles: make([]*Profile, 0),
		},
		frameIndex: make(map[string]uint),
	}

	return obj
}

// SetFlameName 设置当前flame的名字
func (f *FlameGraph) SetFlameName(Name string) {
	f.profileRecord.Name = Name
}

// CreateProfile 创建Profile
func (f *FlameGraph) CreateProfile(Type, Name, Unit string) *Profile {
	obj := &Profile{
		Type:   Type,
		Name:   Name,
		Unit:   Unit,
		parent: f,
	}
	f.profileRecord.Profiles = append(f.profileRecord.Profiles, obj)
	// 返回当前的那个对象指针
	return obj
}

// OpenFrame 开启一个调用栈
func (p *Profile) OpenFrame(Frame Frame, At float64) {
	f := p.parent

	// 获取当前帧对应的索引
	idx, exists := f.frameIndex[Frame.Name]
	if !exists {
		// 加入shared
		f.profileRecord.Shared.Frames = append(f.profileRecord.Shared.Frames, Frame)
		idx = uint(len(f.profileRecord.Shared.Frames) - 1)
		// 加入到map
		f.frameIndex[Frame.Name] = idx
	}

	// 插入到Event
	p.Events = append(p.Events, event{
		At:    At,
		Type:  "O",
		Frame: idx,
	})

	// 更新当前开始时间
	if p.StartValue == 0 {
		p.StartValue = At
	}

	if p.StartValue > At {
		p.StartValue = At
	}
}

// CloseFrame 结束一个调用栈
func (p *Profile) CloseFrame(Name string, At float64) {
	f := p.parent

	// 获取当前帧对应的索引
	idx, exists := f.frameIndex[Name]
	if !exists {
		panic(errors.New("Frame Not Started"))
	}

	// 插入到Event
	p.Events = append(p.Events, event{
		At:    At,
		Type:  "C",
		Frame: idx,
	})

	// 更新当前开始时间
	if p.EndValue < At {
		p.EndValue = At
	}
}

// Marshal 生成JSON
func (f *FlameGraph) Marshal() (string, error) {
	content, err := json.Marshal(f.profileRecord)
	if err != nil {
		return "", err
	}
	return string(content), nil
}
