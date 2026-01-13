cmake_minimum_required(VERSION 3.16)
project(StunadTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCASCADE_DIR "C:/Pratyush/stunad-engine/external/opencascade/build/install/cmake")

find_package(OpenCASCADE REQUIRED)

add_executable(stunad_test main.cpp)

target_include_directories(stunad_test PRIVATE
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(stunad_test PRIVATE
    ${OpenCASCADE_LIBRARIES}
)
