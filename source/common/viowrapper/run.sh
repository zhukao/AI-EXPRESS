rm -rf ./build/bin/libcam.so
rm -rf ./build/bin/libvio.so
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./build/bin/
# ./build/bin/VIOWrapper_unit_test
#./build/bin/VIOWrapper_unit_test --gtest_filter=VIO_WRAPPER_TEST.GetSingleImage_1080
./build/bin/VIOWrapper_unit_test --gtest_filter=VIO_WRAPPER_TEST.GetFbImage_1080
