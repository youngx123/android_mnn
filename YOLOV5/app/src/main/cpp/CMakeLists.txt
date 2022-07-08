# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(MNN_DIR E:/MNN_code)
# mnn的头文件
include_directories(${MNN_DIR}/include)
include_directories(${MNN_DIR}/include/MNN)
include_directories(${MNN_DIR}/tools)
include_directories(${MNN_DIR}/tools/cpp)
include_directories(${MNN_DIR}/source)
include_directories(${MNN_DIR}/source/backend)
include_directories(${MNN_DIR}/source/core)
# opencv的头文件
include_directories(E:/Android_Project/YOLOV5/app/includes)
# 这个是自己定义的.h文件
include_directories(yoloInference.h)
# 链接mnn的动态库，这里编译的是64位的，对应Android里面的arm64-v8a架构
aux_source_directory(. SRCS)

add_library( # Sets the name of the library.
             native-lib
             SHARED
             ${SRCS})

# 需要把libMNN.so放到工程文件里来，具体位置在 app/libs下，放在工程外好像不行
set(dis_DIR E:/Android_Project/YOLOV5/app/libs)
add_library(MNN SHARED IMPORTED)
set_target_properties(MNN PROPERTIES IMPORTED_LOCATION ${dis_DIR}/arm64-v8a/libMNN.so)

add_library(OPENCV SHARED IMPORTED)
set_target_properties(OPENCV PROPERTIES IMPORTED_LOCATION ${dis_DIR}/arm64-v8a/libopencv_java4.so)


find_library( # Sets the name of the path variable.
              log-lib
              log )

target_link_libraries( # Specifies the target library.
                    native-lib
                    ${log-lib}
                    MNN
                    OPENCV
                    jnigraphics)