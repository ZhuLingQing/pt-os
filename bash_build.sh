#for no cmake installed.

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
NCOLOR=$(tput sgr0)

set -e
set -o pipefail
lib_name=pt_os
build_dir=build

echo "----Check g++ has been installed"
git --version

echo "----remove the build dir and make it."
if [ -d ${build_dir} ];then
    rm -rf ${build_dir}/*
else
    mkdir ${build_dir}
fi
pushd ${build_dir}

echo "----clone the dependency repo"
if [ ! -d "protothreads" ];then
    git clone https://github.com/ZhuLingQing/protothreads.git
fi

if [ ! -d "etl" ];then
    git clone https://github.com/ETLCPP/etl.git
    pushd etl
    git checkout 20.38.10
    popd
fi

echo "----build the os static library"
g++ -g -o ${pt-os}.o -c ../pt-os.cpp -I./protothreads -I./etl/include
g++ -g -o ${os-timer}.o -c ../os-timer.cpp -I./protothreads -I./etl/include
ar -rv lib${lib_name}.a ${pt-os}.o ${os-timer}.o 

echo "----build the test"
g++ -g -o test_pt_os ../tests/*.cpp -I./protothreads -I.. -I../tests -L. -l${lib_name}

echo "----run the test"
./test_pt_os
rc=$?
if [ ${rc} -eq 0 ];then
    echo "${GREEN}test pass${NCOLOR}"
else
    echo "${RED}test failed ${rc} ${NCOLOR}"
fi

popd
