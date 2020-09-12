#!/bin/bash
mkdir build
cd build
cmake ..
make
cp ../config/a_filter.json ./bin
cp ../config/b_filter.json ./bin
cp ../config/filter_workflow.json ./bin
