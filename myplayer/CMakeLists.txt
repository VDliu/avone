
cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/include)
#soundtouch 用于对音频进行变速 变调，可单独改变音调和速度
#opensels 变速的时候也改变了音调 因此不好
include_directories(src/main/cpp/soundtouch/include)
include_directories(src/main/cpp/soundtouch/SoundTouch)
# add ffmpeg libs begin
add_library(avcodec-57 SHARED IMPORTED)
set_target_properties( avcodec-57
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)

add_library(avfilter-6 SHARED IMPORTED)
set_target_properties( avfilter-6
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)

add_library( avformat-57 SHARED IMPORTED)
set_target_properties( avformat-57
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)

add_library( avutil-55 SHARED IMPORTED)
set_target_properties( avutil-55
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)

add_library( swresample-2 SHARED IMPORTED)
set_target_properties( swresample-2
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)


add_library( swscale-4 SHARED IMPORTED)
set_target_properties( swscale-4
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)

#add_library( postproc SHARED IMPORTED)
#set_target_properties( postproc
#                       PROPERTIES IMPORTED_LOCATION
#                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc.so)

# add ffmpeg libs end

add_library(
             native-lib
             SHARED
             src/main/cpp/common/MyAudio.cpp
             src/main/cpp/common/MyFFmpeg.cpp
             src/main/cpp/common/AVPacketQueue.cpp
             src/main/cpp/common/PlayStatus.cpp
             src/main/cpp/androidplatform/callback/CallJava.cpp
             #soundtouch begin
             src/main/cpp/soundtouch/SoundTouch/AAFilter.cpp
             src/main/cpp/soundtouch/SoundTouch/FIFOSampleBuffer.cpp
             src/main/cpp/soundtouch/SoundTouch/FIRFilter.cpp
             src/main/cpp/soundtouch/SoundTouch/cpu_detect_x86.cpp
             src/main/cpp/soundtouch/SoundTouch/sse_optimized.cpp
             src/main/cpp/soundtouch/SoundTouch/RateTransposer.cpp
             src/main/cpp/soundtouch/SoundTouch/SoundTouch.cpp
             src/main/cpp/soundtouch/SoundTouch/InterpolateCubic.cpp
             src/main/cpp/soundtouch/SoundTouch/InterpolateLinear.cpp
             src/main/cpp/soundtouch/SoundTouch/InterpolateShannon.cpp
             src/main/cpp/soundtouch/SoundTouch/TDStretch.cpp
             src/main/cpp/soundtouch/SoundTouch/PeakFinder.cpp
             #soundtouch end
             src/main/cpp/native-lib.cpp )



find_library(log-lib
              log )



target_link_libraries(
                       native-lib
                       avcodec-57
                       avfilter-6
                       avformat-57
                       avutil-55
                       swresample-2
                       swscale-4
                       OpenSLES
                       ${log-lib} )