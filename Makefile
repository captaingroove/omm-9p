RELEASE      = 0
RENDER_BACK  = RENDER_VLC
# RENDER_BACK  = RENDER_DUMMY
# RENDER_BACK  = RENDER_FFMPEG

WD           = $(shell pwd)
SYS          = $(WD)/sys
# SYS          = sys
P            = $(SYS)/pkg
B            = build
DVB          = dvb
EXT          = ext

9PLIBS       = -l9 -l9p -l9pclient -lthread -lsec -lmp -lauth -lbio
9PCLIENTLIBS = -l9 -l9pclient -lbio -lsec -lauth -lthread
AVFLAGS      =
AVLIBS       = -lavutil -lavformat -lavcodec -lswscale -lswresample
POCOCFLAGS   = -DPOCO_VERSION_HEADER_FOUND
POCOLIBS     = -lPocoFoundation -lPocoUtil -lPocoXML -lPocoZip
SDL2LIBS     = $(shell pkg-config --libs sdl2)
SDL2CFLAGS   = $(shell pkg-config --cflags sdl2)
SQLITE3LIBS  = $(shell pkg-config --libs sqlite3)

# CFLAGS       = -std=c99 -Wall -Wextra -Wpedantic -Wno-sizeof-array-div
DVBCXXFLAGS  = -g -Wno-format-security -Wno-return-type
DVBLIBS      = $(POCOLIBS) -ludev
PKG_CONFIG   = pkg-config
VLC_PLUGIN_CFLAGS = $(shell $(PKG_CONFIG) --cflags vlc-plugin)
VLC_PLUGIN_LIBS   = $(shell $(PKG_CONFIG) --libs vlc-plugin)
VLCFLAGS     = $(shell $(PKG_CONFIG) --cflags libvlc)
VLCLIBS      = $(shell $(PKG_CONFIG) --libs libvlc)

ifeq ($(RENDER_BACK), RENDER_FFMPEG)
RENDERFLAGS  = $(AVFLAGS)
RENDERLIBS   = $(AVLIBS)
else ifeq ($(RENDER_BACK), RENDER_VLC)
RENDERFLAGS  = $(VLCFLAGS)
RENDERLIBS   = $(VLCLIBS)
endif

ifeq ($(RELEASE), 1)
CFLAGS       = -O2 -DNDEBUG -I$(SYS)/include -I$(SYS)/include/p9
LDFLAGS      = -L$(SYS)/lib
# LDFLAGS      = -static
else
CFLAGS       = -std=c99 -Wall -g -D__DEBUG__ -Wno-sizeof-array-div -I$(SYS)/include -I$(SYS)/include/p9
DBG_CFLAGS   = -fsanitize=address -fsanitize=undefined # -fsanitize=thread
LDFLAGS      = -Wl,-rpath,$(B):$(LD_RUN_PATH) -L$(SYS)/lib
DBG_LDFLAGS  = -lasan -lubsan # -fsanitize=thread
endif

CFLAGS      += -D$(RENDER_BACK)
# LD_RUN_PATH += $(SYS)/lib

## Honor local include and linker paths
export $(CPATH)
export $(LIBRARY_PATH)
## If -rpath is not used as a linker flag, LD_RUN_PATH is honored (we use -rpath for $(B))
## ... sometimes it needs to be exported, sometimes not ...
export $(LD_RUN_PATH)

.PHONY: clean sloc transponder.zip

libixp_url = https://github.com/0intro/libixp.git
p9light_url = https://github.com/captaingroove/p9light.git

$(EXT)/%: $(B)
	cd $(EXT) && git clone $($*_url)

DVBOBJS = \
$(B)/dvb.o \
$(B)/Sys.o \
$(B)/AvStream.o \
$(B)/Descriptor.o \
$(B)/Device.o \
$(B)/Log.o \
$(B)/Section.o \
$(B)/Stream.o \
$(B)/Service.o \
$(B)/Transponder.o \
$(B)/Frontend.o \
$(B)/Demux.o \
$(B)/Remux.o \
$(B)/Dvr.o \
$(B)/TransportStream.o \
$(B)/ElementaryStream.o \
$(B)/TransponderData.o

LIBIXPCFLAGS = -Wno-parentheses -Wno-comment
LIBIXPSRCS = client.c convert.c error.c map.c message.c request.c \
rpc.c server.c socket.c srv_util.c thread.c timer.c transport.c util.c
LIBIXPOBJS = client.o convert.o error.o map.o message.o request.o \
rpc.o server.o socket.o srv_util.o thread.o timer.o transport.o util.o

$(B)/%.o: $(DVB)/%.cpp
	$(CXX) -c -o $@ -I$(B) $(POCOCFLAGS) $(CPPFLAGS) -fPIC $(DVBCXXFLAGS) $<

all: cscope.out $(B) $(P)/p9light $(B)/ommrender $(B)/ommserve $(B)/ommcontrol $(B)/ommscan $(B)/tunedvbcpp $(B)/scandvbcpp $(B)/tunedvb $(B)/libvlc_acc9p_plugin.so

$(B):
	mkdir -p $(B) $(EXT) $(SYS) $(P)

clean:
	rm -rf $(B)
	rm -f $(SYS)/bin/* $(SYS)/lib/* $(SYS)/pkg/*

$(B)/ommcontrol: control.c $(SYS)/lib/libixp.a
	mkdir -p $(B)
	$(CC) -o $@ $< -g -D__DEBUG__ -I$(SYS)/include -L$(SYS)/lib -lixp

$(B)/ommrender: render.c render_ffmpeg.c render_vlc.c
	$(CC) -o $@ $< $(CFLAGS) $(RENDERFLAGS) $(SDL2CFLAGS) $(LDFLAGS) $(9PLIBS) $(RENDERLIBS) $(SDL2LIBS) -lz -lm

$(B)/render: testlab/render.c
	$(CC) -o $@ $< $(CFLAGS) $(SDL2CFLAGS) $(VLCFLAGS) $(LDFLAGS) $(SDL2LIBS) $(VLCLIBS) -lz -lm

$(B)/ommserve: serve.c $(B)/libommdvb.so # $(B)/libommdvb.a
	$(CC) -o $(B)/ommserve $(CFLAGS) $(SDL2CFLAGS) $(LDFLAGS) $< $(9PLIBS) $(SQLITE3LIBS) -L$(B) -lommdvb -lm

$(SYS)/lib/libixp.a:
	cd ext/libixp/lib/libixp && $(CC) -I../../include -fPIC -g -D__DEBUG__ $(LIBIXPCFLAGS) -c $(LIBIXPSRCS)
	cd ext/libixp/lib/libixp && $(AR) rcs libixp.a $(LIBIXPOBJS)
	cp ext/libixp/include/ixp.h $(SYS)/include
	mv ext/libixp/lib/libixp/libixp.a $@
	rm -f ext/libixp/lib/libixp/*.o

$(P)/p9light:
	cd ext/p9light && make && make install PREFIX=$(SYS) && make clean
	touch $@

$(B)/ommscan: scan.c
	$(CC) -o $@ -Wno-deprecated-declarations $(CFLAGS) $(VLCFLAGS) $(LDFLAGS) $< $(VLCLIBS) $(SQLITE3LIBS) -lm

$(B)/resgen: $(B)/resgen.o
	$(CXX) -o $(B)/resgen $< $(POCOLIBS) -lm

$(B)/TransponderData.h: $(B)/resgen transponder.zip
	$(B)/resgen --output-directory=$(DVB) --resource-name=TransponderData $(DVB)/transponder.zip

$(B)/libommdvb.so: $(DVB)/TransponderData.h $(DVBOBJS)
	$(CXX) -o $@ $(DVBOBJS) -shared $(DVBLIBS) -lm

# $(B)/libommdvb.a: $(DVB)/TransponderData.h $(DVBOBJS)
	# $(CXX) -static -o $@ $(DVBOBJS) $(DVBLIBS) -lm

$(B)/tunedvbcpp: $(B)/TuneDvb.o $(B)/libommdvb.so # $(B)/libommdvb.a
	$(CXX) -o $(B)/tunedvbcpp $< $(DVBLIBS) -L$(B) -lommdvb -lm

$(B)/scandvbcpp: $(B)/ScanDvb.o $(B)/libommdvb.so # $(B)/libommdvb.a
	$(CXX) -o $(B)/scandvbcpp $< $(DVBLIBS) -L$(B) -lommdvb -lm

$(B)/tunedvb: $(DVB)/tunedvb.c $(B)/libommdvb.so # $(B)/libommdvb.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ -L$(B) -lommdvb -lm

$(B)/libvlc_acc9p_plugin.so: vlc_acc9p_plugin.c $(SYS)/lib/libixp.a
	$(CC) -o $@ -I$(SYS)/include $(VLC_PLUGIN_CFLAGS) $(LDFLAGS) -shared -fPIC $< -L$(SYS)/lib -lixp $(VLC_PLUGIN_LIBS)

sloc:
	cloc *.c *.h $(DVB)/*.cpp $(DVB)/*.h

cscope.out: *.c *.h $(DVB)/*.cpp $(DVB)/*.h
	cscope -b *.c *.h $(DVB)/*.cpp $(DVB)/*.h
