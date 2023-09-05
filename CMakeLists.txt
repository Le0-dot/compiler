cmake_minimum_required(VERSION 3.20)

project(
    compiler
    VERSION 0.1.0
)

# any_tree

add_subdirectory(include/any_tree)

# nlohmann_json

find_package(nlohmann_json CONFIG REQUIRED)

# llvm options

find_package(LLVM CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(HandleLLVMOptions)

add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs Support Core IRReader)


# Add executable

set(SRC src/main.cpp src/scope.cpp src/functions.cpp src/tree.cpp src/semantic_analyzer.cpp src/type/registry.cpp)

add_executable(${PROJECT_NAME} ${SRC} ${TYPES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${LLVM_INCLUDE_DIRS})

include(CMakePrintHelpers)
cmake_print_variables(llvm_libs)

target_link_libraries(${PROJECT_NAME} PRIVATE ${llvm_libs} any_tree nlohmann_json::nlohmann_json)
