cmake_minimum_required(VERSION 3.16)
project(StunadEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OpenCASCADE_DIR "C:/Pratyush/stunad-engine/external/opencascade/build/install/cmake")
find_package(OpenCASCADE REQUIRED)




add_subdirectory(engine)
add_subdirectory(test)
