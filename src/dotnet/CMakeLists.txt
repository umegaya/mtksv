cmake_minimum_required(VERSION 3.0)
# get mono build settings
set(MONO_CFLAGS "-D_REENTRANT -I/usr/local/lib/pkgconfig/../../include/mono-2.0")
set(MONO_LDFLAGS "-L/usr/local/lib/pkgconfig/../../lib")

# load common settings
include(${CMAKE_CURRENT_SOURCE_DIR}/../base.cmake)

# modify compile options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MONO_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MONO_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MONO_LDFLAGS}")

# populate files
file(GLOB_RECURSE libsrc [
	# lib source
	"./*.cpp" 
])
set(src ${libsrc} ${coresrc})

# add static libs
set(libroot "/usr/local/lib")
set(glibroot "/usr/lib/x86_64-linux-gnu")
add_library(mtk STATIC IMPORTED)
add_library(mono-2.0 STATIC IMPORTED)
add_library(z STATIC IMPORTED)
set_target_properties(mtk PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/mtk/build/linux/libmtk.a")
set_target_properties(mono-2.0 PROPERTIES IMPORTED_LOCATION "${libroot}/libmono-2.0.a")
set_target_properties(z PROPERTIES IMPORTED_LOCATION "${glibroot}/libz.a")

# build executable
add_executable(mtkdn ${src}) 	
target_link_libraries(mtkdn mtk mono-2.0 pthread dl z m rt -static-libstdc++ -static-libgcc)
