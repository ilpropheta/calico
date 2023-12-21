set(CMAKE_CXX_STANDARD 20)
set(TargetName calico)

FILE(GLOB CppSources *.cpp *.cc */*.cpp)
add_executable (${TargetName} ${CppSources})
target_link_libraries(${TargetName} PRIVATE calico_core)