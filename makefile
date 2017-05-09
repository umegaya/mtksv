SERVER_ROOT=../../src
VERBOSE=0


# build dotnet server
mono: 
	cd ext/mono && ./autogen.sh --disable-nls && make && make install

builder-dotnet:
	docker run --name builder-dotnet -ti -v `pwd`:/mtksv mtktools/builder bash -c "cd mtksv && make mono"
	docker commit builder-dotnet mtktools/bulder-dotnet
	docker kill builder-dotnet && doker rm builder-dotnet

compile-dotnet: 
	make -C ext/mtk linux
	- mkdir -p build/dotnet
	cd build/dotnet && cmake $(SERVER_ROOT)/dotnet && make

build-dotnet:
	docker run --rm -ti -v `pwd`:/mtksv mtktools/builder-dotnet bash -c "cd mtksv && make compile-dotnet"

image-dotnet:
	echo "TODO"