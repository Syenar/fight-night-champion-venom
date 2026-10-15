cmake_minimum_required(VERSION 3.20)
project(fnc_runtime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(fnc_runtime STATIC src/fnc_runtime.cpp)
target_include_directories(fnc_runtime PUBLIC include)

add_executable(fnc_runtime_contract_test tests/runtime_contract_test.cpp)
target_link_libraries(fnc_runtime_contract_test PRIVATE fnc_runtime)
enable_testing()
add_test(NAME fnc_runtime_contract_test COMMAND fnc_runtime_contract_test)
