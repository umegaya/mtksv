SERVER_SRC_ROOT=../../src
BUILDER_PREFIX=mtktool/builder
BUILDER_BUILD_DIR=container/builder
RUNTIME_PREFIX=mtktool/runtime
RUNTIME_BUILD_DIR=container/runtime
IMAGE_BUILD_DIR=container/image

# build dotnet server
mtk:
	make -C ext/mtk linux

builder-dotnet:
	docker build -t $(BUILDER_PREFIX)-dotnet $(BUILDER_BUILD_DIR)/dotnet

compile-dotnet: mtk
	- mkdir -p build/dotnet
	cd build/dotnet && cmake $(SERVER_SRC_ROOT)/dotnet && make

DOTNET_OUT=build/dotnet
runtime-dotnet:
	docker run --rm -ti -v `pwd`:/mtksv $(BUILDER_PREFIX)-dotnet bash -c "cd mtksv && make compile-dotnet"
	@mv $(DOTNET_OUT)/mtkdn $(RUNTIME_BUILD_DIR)/dotnet
	@mv $(DOTNET_OUT)/stub/libserver.so $(RUNTIME_BUILD_DIR)/dotnet
	docker build -t mtktool/runtime-dotnet $(RUNTIME_BUILD_DIR)/dotnet

image-dotnet:
	@mkdir -p $(DOTNET_OUT)/server
	docker run --rm -ti -v $(SVDIR):/codes/Server -v `pwd`/ext/mtk/bindings/csharp:/codes/Mtk -v `pwd`/tools/dotnet:/mtk/bin -v `pwd`/$(DOTNET_OUT)/server:/tmp/out \
		$(BUILDER_PREFIX)-dotnet make -C /mtk/bin lib
	@mv $(DOTNET_OUT)/server/libserver.so $(IMAGE_BUILD_DIR)/dotnet
	docker build -t $(IMAGE) $(IMAGE_BUILD_DIR)/dotnet