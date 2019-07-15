
cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/include)
# add ffmpeg libs begin
add_library(avcodec SHARED IMPORTED)
set_target_properties( avcodec
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec.so)


add_library( avformat SHARED IMPORTED)
set_target_properties( avformat
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat.so)

add_library( avutil SHARED IMPORTED)
set_target_properties( avutil
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil.so)

# add ffmpeg libs end

add_library(
             ffmpeg-lib
             SHARED
             src/main/cpp/ffmpeg-lib.cpp )



find_library(log-lib
              log )



target_link_libraries(
                      ffmpeg-lib
                      avcodec
                      avformat
                      avutil
                      ${log-lib} )