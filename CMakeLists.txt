cmake_minimum_required(VERSION 3.20)

project(
    compiler
    VERSION 0.1.0
)

# ant_tree

add_subdirectory(any_tree)


# llvm options

find_package(LLVM CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(HandleLLVMOptions)

add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs Support Core IRReader)


# Add executable

set(SRC src/main.cpp src/type/registry.cpp)

add_executable(${PROJECT_NAME} ${SRC} ${TYPES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ PRIVATE ${LLVM_INCLUDE_DIRS})

include(CMakePrintHelpers)
cmake_print_variables(llvm_libs)

target_link_libraries(${PROJECT_NAME} PRIVATE ${llvm_libs} PUBLIC any_tree)
