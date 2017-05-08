SERVER_ROOT=../../src
DOTNET_LIB=

mono: 
	cd ext/mono && ./autogen.sh --disable-nls && make && make install

dnimg:
	docker build -t mtktools/dotnet .

dotnet: 
	make -C ext/mtk linux
	- mkdir -p build/dotnet
	cd build/dotnet && cmake $(SERVER_ROOT)/dotnet && make

