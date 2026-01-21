add_library(stunad_engine STATIC
    occt/OcctKernel.cpp
    occt/OcctTessellator.cpp 
    extra/MeshPostProcess.cpp
    extra/GltfExporter.cpp
    grammar/GrammarExecutor.cpp
    grammar/GrammarBuilder.cpp
    grammar/GrammarValidator.cpp
    ${CMAKE_SOURCE_DIR}/external/tinygltf/tiny_gltf.cc
)

set(OCCT_ROOT ${CMAKE_SOURCE_DIR}/external/occt_sdk/opencascade-7.9.3-vc14-64)

target_include_directories(stunad_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OCCT_ROOT}/inc
    ${CMAKE_SOURCE_DIR}/external/tinygltf
    ${CMAKE_SOURCE_DIR}/external/json/include
)

target_link_directories(stunad_engine PUBLIC
    ${OCCT_ROOT}/win64/vc14/lib
)

target_link_libraries(stunad_engine PUBLIC
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKBO
    TKBool
    TKFillet
    TKOffset
    TKShHealing
    TKMesh
    TKXMesh
    TKXSBase
    TKDE
    TKDESTEP
)

# Copy OCCT DLLs next to the library
add_custom_command(TARGET stunad_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${OCCT_ROOT}/win64/vc14/bin
        $<TARGET_FILE_DIR:stunad_engine>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/external/occt_sdk/3rdparty-vc14-64/tbb-2021.13.0-x64/bin
        $<TARGET_FILE_DIR:stunad_engine>
  COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/external/occt_sdk/3rdparty-vc14-64/jemalloc-vc14-64/bin
        $<TARGET_FILE_DIR:stunad_engine>
 COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/external/occt_sdk/3rdparty-vc14-64/openvr-1.14.15-64/bin/win64
        $<TARGET_FILE_DIR:stunad_engine>
COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/external/occt_sdk/3rdparty-vc14-64/freeimage-3.18.0-x64/bin
        $<TARGET_FILE_DIR:stunad_engine>
 COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/external/occt_sdk/3rdparty-vc14-64/ffmpeg-3.3.4-64/bin
        $<TARGET_FILE_DIR:stunad_engine>

)

