cmake_minimum_required(VERSION 3.0)
include(${CMAKE_CURRENT_SOURCE_DIR}/../base.cmake)
file(GLOB_RECURSE libsrc [
	# lib source
	"./*.cpp" 
])
set(src ${libsrc} ${coresrc})
# add mtk lib
add_library(mtk STATIC IMPORTED)
set_target_properties(mtk PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/mtk/build/linux/libmtk.a")

# executable
add_executable(mtkdn ${src}) 	
target_link_libraries(mtkdn mtk z)
