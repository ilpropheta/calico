set(CMAKE_CXX_STANDARD 20)
set(TargetName calico_tests)

FILE(GLOB CppSources *.cpp *.cc */*.cpp)
add_executable (${TargetName} ${CppSources})

find_package(GTest CONFIG REQUIRED)

file(GLOB DATA_TO_COPY "${CMAKE_SOURCE_DIR}/calico/data/*.*")
file(COPY ${DATA_TO_COPY} DESTINATION ${CMAKE_BINARY_DIR}/calico_tests)

target_link_libraries(${TargetName} PRIVATE calico_core GTest::gmock GTest::gtest)

add_custom_command(
    TARGET ${TargetName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data ${CMAKE_BINARY_DIR}/calico_tests/test_data
    COMMENT "Copying test_data directory"
    VERBATIM
)