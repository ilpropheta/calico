set(CMAKE_CXX_STANDARD 20)
set(TargetName calico_core)

FILE(GLOB CppSources *.cpp *.cc */*.cpp)
add_library (${TargetName} STATIC ${CppSources})

if(WIN32)
  set(SOBJECTIZER_LIB sobjectizer::SharedLib)
else()
  set(SOBJECTIZER_LIB sobjectizer::StaticLib)
endif()

set(OPENCV_LIBS opencv_core opencv_highgui opencv_imgcodecs opencv_imgproc opencv_objdetect)
set(GRPC_LIBS gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 20)
endif()

find_package(sobjectizer CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

file(GLOB DATA_TO_COPY "data/*.*")
file(COPY ${DATA_TO_COPY} DESTINATION ${CMAKE_BINARY_DIR}/calico_main)

if(WIN32)
  set(cv_cap_api "cv::CAP_DSHOW")
elseif(LINUX)
  set(cv_cap_api "cv::CAP_V4L")
else()
  set(cv_cap_api "cv::CAP_ANY")
endif()

target_compile_definitions(${TargetName} PRIVATE CAP_API=${cv_cap_api})

# automatic generation of grpc service files
set(ProtoTargetName proto-objects)
add_library(${ProtoTargetName} OBJECT "${CMAKE_SOURCE_DIR}/protos/service.proto")
target_link_libraries(${ProtoTargetName} PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure gRPC::grpc++_reflection)

set(PROTO_IMPORT_DIRS "${CMAKE_SOURCE_DIR}/protos")
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

protobuf_generate(
    TARGET ${ProtoTargetName}
    OUT_VAR PROTO_GENERATED_FILES
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)

protobuf_generate(
    TARGET ${ProtoTargetName}
    OUT_VAR PROTO_GENERATED_FILES
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)
target_include_directories(${ProtoTargetName} PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

target_link_libraries(${TargetName} PUBLIC ${ProtoTargetName} ${SOBJECTIZER_LIB} ${OPENCV_LIBS} ${GRPC_LIBS})
