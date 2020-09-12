#!/bin/bash
mkdir build
cd build
cmake ..
make
cp ../config/thread_safety_reorder_workflow.json ./bin
