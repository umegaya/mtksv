# common settings
CC=gcc
MCS=mcs
MKBUNDLE=mkbundle
OUTDIR=/tmp/out
LIB=$(OUTDIR)/libserver.so

# build server logic assemly
MCSFLAGS=-unsafe -t:library
CODE=/codes
DEPS=-r:/usr/lib/mono/nuget/Google.Protobuf.3.3.0/lib/net45/Google.Protobuf.dll
MTK_DLL=$(OUTDIR)/Mtk.dll
MTK_SRC=$(shell ls $(CODE)/Mtk/*.cs)
SERVER_DLL=$(OUTDIR)/Server.dll
SERVER_SRC=$(shell find $(CODE)/Server -name '*.cs')

$(MTK_DLL): $(MTK_SRC)
	$(MCS) $(MCSFLAGS) $(DEPS) -out:$@ $(MTK_SRC)

$(SERVER_DLL): $(SERVER_SRC) $(MTK_DLL)
	$(MCS) $(MCSFLAGS) $(DEPS) -r:$(MTK_DLL) -out:$@ $(SERVER_SRC)


# convert server logic assemly to shared library
EMBED_O=$(OUTDIR)/embed.o
EMBED_C=$(OUTDIR)/embed.c
ASSEMBLY=$(OUTDIR)/assembly.o
CFLAG=`pkg-config --cflags --libs mono-2` -fPIC
LDFLAG=--shared -fPIC
MKBUNDLE_FLAGS=-v --custom -c --nomain -L /usr/lib/mono/4.5 -L /usr/lib/mono/nuget/Google.Protobuf.3.3.0/lib/net45

$(LIB): $(EMBED_O) $(ASSEMBLY)
	$(CC) $(LDFLAG) -o $@ $< $(ASSEMBLY)

$(EMBED_O): $(EMBED_C)
	$(CC) $(CFLAG) -c $< -o $@

$(EMBED_C): $(SERVER_DLL)
	cd $(OUTDIR) && $(MKBUNDLE) $(MKBUNDLE_FLAGS) -o $@ -oo $(ASSEMBLY) $< --deps

dll: $(MTK_DLL) $(SERVER_DLL)

lib: dll $(LIB)