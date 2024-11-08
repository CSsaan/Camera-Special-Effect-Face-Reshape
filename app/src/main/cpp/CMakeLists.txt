# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("cameraspecialeffect")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(
        third_party/glm/
        third_party/mnn/include/MNN
        third_party/opencv/include
        #        third_party/openCL/include
        third_party/ncnnvulkan/${ANDROID_ABI}/include/
)

#SET(opencl_lib_dir ${CMAKE_SOURCE_DIR}/third_party/OpenCL/arm64-v8a/libOpenCL.so)
#add_library(libOpenCL STATIC IMPORTED)
#set_target_properties(libOpenCL
#        PROPERTIES IMPORTED_LOCATION ${opencl_lib_dir}
#        )


add_library( # Sets the name of the library.
        cameraspecialeffect
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        native-lib.cpp

        utils/Log.h
        utils/ProgramLoader.cpp

        faces/stb_image.h
        faces/UltraFace.cpp
        faces/Pfld.cpp
        faces/ImageUtils.cpp

        renders/BaseVideoRender.cpp
        renders/FaceReshape.cpp
        renders/FaceReshape2.cpp
        renders/FaceMask.cpp
        renders/ToothWhiten.cpp
        renders/BigEyes.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


add_library(mnn SHARED IMPORTED)
set_target_properties(
        mnn
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/third_party/mnn/${ANDROID_ABI}/libMNN.so
)

set(ncnn_DIR ${CMAKE_SOURCE_DIR}/third_party/ncnnvulkan/${ANDROID_ABI}/lib/cmake/ncnn)
find_package(ncnn REQUIRED)
set_target_properties(
        ncnn
        PROPERTIES
        INTERFACE_COMPILE_OPTIONS "-frtti;-fexceptions"
        # ncnn.cmake 里面是关的，把它重新打开防止跟opencv2冲突，如果是重新编译ncnn的请自己尝试要开还是关
)

add_library(libMNN STATIC IMPORTED)
add_library(libMNN_CL STATIC IMPORTED)
add_library(libMNN_Express STATIC IMPORTED)
set_target_properties(
        libMNN
        libMNN_CL
        libMNN_Express
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/third_party/mnn/${ANDROID_ABI}/libMNN.so
        ${CMAKE_SOURCE_DIR}/third_party/mnn/${ANDROID_ABI}/libMNN_CL.so
        ${CMAKE_SOURCE_DIR}/third_party/mnn/${ANDROID_ABI}/libMNN_Express.so

)

add_library(opencv STATIC IMPORTED)
set_target_properties(
        opencv
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/third_party/opencv/${ANDROID_ABI}/libopencv_java4.so
)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


add_definitions(-DANDROID_STL=c++_shared)
target_link_libraries( # Specifies the target library.
        cameraspecialeffect
        GLESv3  #opengles 3.0
        android # android
#        libOpenCL
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        libMNN libMNN_CL libMNN_Express
        opencv
        ncnn
        )