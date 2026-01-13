cmake_minimum_required(VERSION 3.16)
project(StunadTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCASCADE_DIR "C:/Pratyush/stunad-engine/external/opencascade/build/install/cmake")
find_package(OpenCASCADE REQUIRED)

# OCCT root = one level above the cmake folder
get_filename_component(OCCT_ROOT "${OpenCASCADE_DIR}/.." ABSOLUTE)

set(OCCT_INC "${OCCT_ROOT}/inc")
set(OCCT_LIB "${OCCT_ROOT}/win64/vc14/lib")
set(OCCT_BIN "${OCCT_ROOT}/win64/vc14/bin")

add_executable(stunad_test main.cpp)

target_include_directories(stunad_test PRIVATE
    ${OCCT_INC}
)

target_link_directories(stunad_test PRIVATE
    ${OCCT_LIB}
)

target_link_libraries(stunad_test PRIVATE
    stunad_engine
    ${OpenCASCADE_LIBRARIES}
)

add_custom_command(TARGET stunad_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${OCCT_BIN}"
        $<TARGET_FILE_DIR:stunad_test>
)
