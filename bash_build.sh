#!/bin/bash
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
NCOLOR=$(tput sgr0)

set -e
set -o pipefail
lib_name=pt_os
build_dir=build

#test g++ and git has been installed
g++ --version
git --version

#remove the build dir and make it.
if [ -d ${build_dir} ];then
    rm -rf ${build_dir}/*
else
    mkdir ${build_dir}
fi
pushd ${build_dir}

#clone the dependency repo
git clone https://github.com/ZhuLingQing/protothreads.git
#build the os static library
g++ -g -o ${lib_name}.o -c ../pt-os.cpp -I./protothreads
ar -rv lib${lib_name}.a ${lib_name}.o
#build the test
g++ -g -o test_pt_os ../tests/*.cpp -I./protothreads -I.. -L. -l${lib_name}
#run the test
./test_pt_os
rc=$?
if [ ${rc} -eq 0 ];then
    echo "${GREEN}test pass${NCOLOR}"
else
    echo "${RED}test failed ${rc} ${NCOLOR}"
fi

popd
