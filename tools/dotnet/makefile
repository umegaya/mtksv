# common settings
CC=gcc
MCS=mcs
MKBUNDLE=mkbundle
OUTDIR=/tmp/out
LIB=$(OUTDIR)/libserver.so

# build server logic assemly
MCSFLAGS=-unsafe -t:library -define:MTKSV
CODE=/codes
MTK_DEPS=$(shell find $(CODE)/Mtk/Ext/*/lib/net4* -name '*.dll')
MTK_DEPS_FLAGS=$(addprefix -r:,$(MTK_DEPS))
MTK_DLL=$(OUTDIR)/Mtk.dll
MTK_SRC=$(shell ls $(CODE)/Mtk/*.cs)
SERVER_DLL=$(OUTDIR)/Server.dll
SERVER_SRC=$(shell find $(CODE)/Server -name '*.cs')

$(MTK_DLL): $(MTK_SRC)
	$(MCS) $(MCSFLAGS) $(MTK_DEPS_FLAGS) -out:$@ $(MTK_SRC)

$(SERVER_DLL): $(SERVER_SRC) $(MTK_DLL)
	$(MCS) $(MCSFLAGS) -r:$(MTK_DLL) $(MTK_DEPS_FLAGS) -out:$@ $(SERVER_SRC)


# convert server logic assemly to shared library
EMBED_O=$(OUTDIR)/embed.o
EMBED_C=$(OUTDIR)/embed.c
ASSEMBLY=$(OUTDIR)/assembly.o
CFLAG=`pkg-config --cflags --libs mono-2` -fPIC
LDFLAG=--shared -fPIC
MKBUNDLE_FLAGS=-v --custom -c --nomain -L /usr/lib/mono/4.5 $(addprefix -L ,$(dir $(MTK_DEPS)))

$(LIB): $(EMBED_O) $(ASSEMBLY)
	$(CC) $(LDFLAG) -o $@ $< $(ASSEMBLY)

$(EMBED_O): $(EMBED_C)
	$(CC) $(CFLAG) -c $< -o $@

$(EMBED_C): $(SERVER_DLL)
	cd $(OUTDIR) && $(MKBUNDLE) $(MKBUNDLE_FLAGS) -o $@ -oo $(ASSEMBLY) $< --deps

dll: $(MTK_DLL) $(SERVER_DLL)

lib: dll $(LIB)