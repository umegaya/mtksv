BUILDER_IMG=mtktool/builder
DOTNET_OUT=build/server/dotnet

# build dotnet server
mono: 
	cd ext/mono && ./autogen.sh --disable-nls && make && make install
	apt-get update && apt-get install -y --no-install-recommends ca-certificates-mono
	cert-sync /etc/ssl/certs/ca-certificates.crt && rm -rf /var/lib/apt/lists/*
	mkdir -p /usr/local/lib/mono/nuget
	curl -o /usr/local/lib/mono/nuget/NuGet.exe -L https://github.com/NuGet/Home/releases/download/3.3/NuGet.exe
	mkdir -p /tmp/mtk-tools && cp /mtksv/tools/dotnet/* /tmp/mtk-tools/ && chmod 755 /tmp/mtk-tools/*
	mv /tmp/mtk-tools/* /usr/local/bin/ && rm -r /tmp/mtk-tools
	nuget config -set repositoryPath=/usr/local/lib/mono/nuget
	nuget install Google.Protobuf

builder-dotnet:
	docker run --name builder-dotnet -ti -v `pwd`:/mtksv $(BUILDER_IMG) bash -c "cd mtksv && make mono"
	docker commit builder-dotnet $(BUILDER_IMG)-dotnet
	docker kill builder-dotnet && doker rm builder-dotnet

compile-dotnet: 
	make -C ext/mtk linux
	- mkdir -p build/dotnet
	cd build/dotnet && cmake $(SERVER_ROOT)/dotnet && make

runtime-dotnet:
	docker run --rm -ti -v `pwd`:/mtksv $(BUILDER_IMG)-dotnet bash -c "cd mtksv && make compile-dotnet"
	@echo "TODO: pack result binary into smaller container"

image-dotnet:
	mkdir -p `pwd`/$(DOTNET_OUT)
	docker run --rm -ti -v $(SVDIR):/codes/Server \
		-v `pwd`/ext/mtk/bindings/csharp:/codes/Mtk -v `pwd`/tools/dotnet:/mtk/bin -v `pwd`/$(DOTNET_OUT):/tmp/out \
		$(BUILDER_IMG)-dotnet bash /mtk/bin/mksv /tmp/out
	@echo "create container which adds $(DOTNET_OUT)/Server.dll into mtktools/runtime-dotnet"
