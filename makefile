SERVER_ROOT=../../src

dotnet: 
	make -C ext/mtk linux
	- mkdir -p build/dotnet
	cd build/dotnet && cmake $(SERVER_ROOT)/dotnet && make

