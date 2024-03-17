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
git clone https://github.com/ETLCPP/etl.git
pushd etl
git checkout 20.38.10
popd
#build the os static library
g++ -g -o ${pt-os}.o -c ../pt-os.cpp -I./protothreads -I./etl
g++ -g -o ${os-timer}.o -c ../os-timer.cpp -I./protothreads -I./etl
g++ -g -o ${os-timer-port}.o -c ../os-timer-port.cpp -I./protothreads -I./etl
ar -rv lib${lib_name}.a ${pt-os}.o ${os-timer}.o  ${os-timer-port}.o 
#build the test
g++ -g -o test_pt_os ../tests/*.cpp -I./protothreads -I.. -I../tests -L. -l${lib_name}
#run the test
./test_pt_os
rc=$?
if [ ${rc} -eq 0 ];then
    echo "${GREEN}test pass${NCOLOR}"
else
    echo "${RED}test failed ${rc} ${NCOLOR}"
fi

popd
