set(DEBUG false CACHE BOOL "do debug build")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DHAVE_PTHREAD")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -DOPENSSL_NO_ASM -DPB_NO_PACKED_STRUCTS")
set(EXTPATH ${CMAKE_CURRENT_SOURCE_DIR}/../../ext)
include_directories(src 
	${EXTPATH}
	${EXTPATH}/mtk/src/proto/src
	${CMAKE_CURRENT_SOURCE_DIR}/../core)
include_directories(SYSTEM 
	${EXTPATH}/mtk/ext
	${EXTPATH}/mtk/ext/spdlog/include
	${EXTPATH}/mtk/ext/grpc/include 
	${EXTPATH}/mtk/ext/grpc/third_party/boringssl/include 
	${EXTPATH}/mtk/ext/grpc/third_party/protobuf/src)
if (DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -DDEBUG")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -DDEBUG")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")
endif()	

file(GLOB_RECURSE coresrc [
	# lib source
	"${CMAKE_CURRENT_SOURCE_DIR}/../core/*.cpp" 
])