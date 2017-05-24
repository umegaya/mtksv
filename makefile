SERVER_SRC_ROOT=../../src
BUILDER_PREFIX=mtktool/builder
BUILDER_BUILD_DIR=container/builder
RUNTIME_PREFIX=mtktool/runtime
RUNTIME_BUILD_DIR=container/runtime
IMAGE_BUILD_DIR=container/image

# common function
define build_builder
	docker build -t $(BUILDER_PREFIX)-$1 $(BUILDER_BUILD_DIR)/$1
endef

define build_runtime
	docker run --rm -ti -v `pwd`:/mtksv $(BUILDER_PREFIX)-$1 bash -c "cd mtksv && make compile-$1"
	bash $(RUNTIME_BUILD_DIR)/$1/setup.sh
	docker build -t $(RUNTIME_PREFIX)-$1 $(RUNTIME_BUILD_DIR)/$1
endef

define build_image
	bash $(IMAGE_BUILD_DIR)/$1/build.sh $3
	docker build -t $2 $(IMAGE_BUILD_DIR)/$1
endef

define compile
	- mkdir -p build/$1
	cd build/$1 && cmake $(SERVER_SRC_ROOT)/$1 && make
endef


# build base runtime
mtk:
	make -C ext/mtk linux


# build dotnet server
builder-dotnet:
	$(call build_builder,dotnet)

compile-dotnet: 
	$(call compile,dotnet)

runtime-dotnet:
	$(call build_runtime,dotnet)

image-dotnet:
	$(call build_image,dotnet,$(IMAGE),$(SVDIR))

shell-dotnet:
	docker run --rm -ti -v `pwd`:/mtksv --privileged mtktool/builder-dotnet bash


# build coreclr server
builder-msdn:
	$(call build_builder,msdn)

compile-msdn: mtk
	$(call compile,msdn)

runtime-msdn:
	$(call build_runtime,msdn)

image-msdn:
	$(call build_image,msdn,$(IMAGE),$(SVDIR))
