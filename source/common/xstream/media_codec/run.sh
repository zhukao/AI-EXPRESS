#!/bin/sh

export LD_LIBRARY_PATH=../../../../build/lib:$LD_LIBRARY_PATH

if [ ! -d "bin" ]; then
    mkdir ./bin
fi

jpg_files=$(ls ./data/*.jpg 2>/dev/null | wc -l)
if [ "$jpg_files" != "0" ]; then
    rm data/*.jpg
fi
cp ../../../../build/bin/media_codec_test ./bin
# -h is params help document
# ./bin/media_codec_test -h
./bin/media_codec_test
# ./bin/media_codec_test --gtest_filter=MEDIA_CODEC_TEST.nv12_to_jpeg -W 1920 -H 1080 -F nv12 -I ./data/ -i input_nv12_1080p.yuv -D 5 -C 1 -N 3 -Q 50
# ./bin/media_codec_test --gtest_filter=MEDIA_CODEC_TEST.yuv420p_to_jpeg
# ./bin/media_codec_test --gtest_filter=MEDIA_CODEC_TEST.yuyv422_to_jpeg
# ./bin/media_codec_test --gtest_filter=MEDIA_CODEC_TEST.yuv422p_to_jpeg
