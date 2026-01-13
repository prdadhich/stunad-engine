message(STATUS "OpenCASCADE_DIR = ${OpenCASCADE_DIR}")
get_filename_component(OCCT_ROOT "${OpenCASCADE_DIR}/.." ABSOLUTE)
message(STATUS "OCCT_ROOT = ${OCCT_ROOT}")
message(STATUS "OCCT_INC = ${OCCT_ROOT}/inc")


get_filename_component(OCCT_ROOT "${OpenCASCADE_DIR}/.." ABSOLUTE)

set(OCCT_INC "${OCCT_ROOT}/inc")
set(OCCT_LIB "${OCCT_ROOT}/win64/vc14/lib")

add_library(stunad_engine STATIC
    occt/OcctKernel.cpp
)

target_include_directories(stunad_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OCCT_INC}
)

target_link_directories(stunad_engine PUBLIC
    ${OCCT_LIB}
)

target_link_libraries(stunad_engine PUBLIC
    ${OpenCASCADE_LIBRARIES}
)
