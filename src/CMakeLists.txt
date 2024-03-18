set(BINDINGS _fastgdal2tiles)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_FILES
    ${SRC_DIR}/fast_tile_bindings.cpp
    ${SRC_DIR}/src/fasttiler.cpp
    ${SRC_DIR}/src/png_io.cpp
    ${SRC_DIR}/src/tile_fns.cpp
    ${SRC_DIR}/third_party/fpng/fpng.cpp
)

nanobind_add_module(${BINDINGS} ${SRC_FILES})
target_link_libraries(${BINDINGS} PRIVATE ${GDAL_LIBRARIES})
install(TARGETS ${BINDINGS} DESTINATION fastgdal2tiles)

