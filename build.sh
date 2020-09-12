#!/bin/bash
set -x

function go_build_all(){
  rm build -rf
  mkdir build
  cd build
  cmake .. $*
  make -j2
  if [ $? -ne 0 ] ; then
    echo "failed to build "
    exit 1
  fi
  make install
}

usage(){
  echo "usage: bash build.sh [x2|x3]"
  exit 1
}

# check make.sh parameters
if [ $# -ge 1 ]
then
  ARCHITECTURE=${1}
  echo "${1}" > platform.tmp
else
  echo "error!! architecture must be specified."
  usage
fi

ARCHITECTURE="x2"

if [ ${1} == "x2" ]
then
  ARCHITECTURE="x2"
elif [ ${1} == "x3" ]
then
  ARCHITECTURE="x3"
else
  echo "${1} architecture is not supported."
  usage
fi

OUTPLG="web"
if [ $# -ge 2 ]
then
  OUTPLG_HBIPC=${2}
fi

if [ ${ARCHITECTURE} == "x2" ]; then
  if [ ${OUTPLG_HBIPC} == "hbipc" ]; then
    go_build_all -DOUTPLG_HBIPC=ON
  else
    go_build_all
  fi
elif [ ${ARCHITECTURE} == "x3" ]; then
    go_build_all -DPLATFORM_X3=ON -DPLATFORM_X2=OFF
fi
