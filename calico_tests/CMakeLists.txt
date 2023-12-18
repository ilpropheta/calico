set(CMAKE_CXX_STANDARD 20)
set(TargetName calico_tests)

FILE(GLOB CppSources *.cpp *.cc */*.cpp)
add_executable (${TargetName} ${CppSources})

find_package(GTest CONFIG REQUIRED)

target_link_libraries(${TargetName} PRIVATE calico_core GTest::gmock GTest::gtest)