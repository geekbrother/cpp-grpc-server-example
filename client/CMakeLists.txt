cmake_minimum_required(VERSION 3.19)
project(basic_grpc)

set(CMAKE_CXX_STANDARD 17)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)

if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

get_filename_component(sample_proto "../protos/sample.proto" ABSOLUTE)
get_filename_component(sample_proto_path "${sample_proto}" PATH)

set(sample_proto_sources "${CMAKE_CURRENT_BINARY_DIR}/sample.pb.cc")
set(sample_proto_headers "${CMAKE_CURRENT_BINARY_DIR}/sample.pb.h")
set(sample_grpc_sources "${CMAKE_CURRENT_BINARY_DIR}/sample.grpc.pb.cc")
set(sample_grpc_headers "${CMAKE_CURRENT_BINARY_DIR}/sample.grpc.pb.h")
add_custom_command(
  OUTPUT "${sample_proto_sources}" "${sample_proto_headers}" "${sample_grpc_sources}" "${sample_grpc_headers}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
  -I "${sample_proto_path}"
  --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
  "${sample_proto}"
  DEPENDS "${sample_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(sample_grpc_proto
  ${sample_proto_sources}
  ${sample_proto_headers}
  ${sample_grpc_sources}
  ${sample_grpc_headers})
target_link_libraries(sample_grpc_proto
  ${_PROTOBUF_LIBPROTOBUF}
  ${_REFLECTION}
  ${_GRPC_GRPCPP})

add_executable(grpc_client "src/client.cpp")
target_link_libraries(grpc_client
  sample_grpc_proto
  ${_PROTOBUF_LIBPROTOBUF}
  ${_REFLECTION}
  ${_GRPC_GRPCPP})
