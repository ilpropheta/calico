﻿set(CMAKE_CXX_STANDARD 20)
set(TargetName calico)

FILE(GLOB CppSources *.cpp *.cc */*.cpp)
add_executable (${TargetName} ${CppSources})

if(WIN32)
  set(SOBJECTIZER_LIB sobjectizer::SharedLib)
else()
  set(SOBJECTIZER_LIB sobjectizer::StaticLib)
endif()

set(OPENCV_LIBS opencv_core opencv_highgui opencv_imgcodecs opencv_imgproc)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)
endif()

find_package(sobjectizer CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

target_link_libraries(${TargetName} PRIVATE ${SOBJECTIZER_LIB} ${OPENCV_LIBS})