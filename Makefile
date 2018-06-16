all : testspread testsurvive testgame

uname_m := $(shell uname -m)


#USE_GPU:=-DMALI
#LINK_GPU:=-lMali

USE_GPU:=-DRASPI_GPU -DGLES2 -I/opt/vc/include # -DNEED_BUFFER_BITS
LINK_GPU:=-L/opt/vc/lib -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm

ifeq ($(uname_m), x86_64)
	CNHTTP:=cntools/http/http_bsd.c cntools/http/cnhttp.c cntools/http/mfs.c cntools/http/sha1.c
	RAWDRAW:=rawdraw/CNFG3D.c rawdraw/CNFGXDriver.c rawdraw/CNFGFunctions.c src/objload.c
	RESOURCE_O:=
	SPREADGINE_C:=src/spreadgine.c src/spreadgine_remote.c $(CNHTTP) $(RAWDRAW)

	CFLAGS:=-O2 -g -Iinclude -Icntools/http -Irawdraw -DCNFGOGL -DHTTP_POLL_TIMEOUT=10 -DCNFG3D_USE_OGL_MAJOR
	LDFLAGS:=-lm -lX11 -lXext -lGL -lpthread
else
	CNHTTP:=cntools/http/http_bsd.o cntools/http/cnhttp.o cntools/http/mfs.o cntools/http/sha1.o
	RAWDRAW:=rawdraw/CNFG3D.o rawdraw/CNFGEGLDriver.o rawdraw/CNFGFunctions.o
	RESOURCE_O:=$(CNHTTP) $(RAWDRAW) src/spreadgine.o src/spreadgine_remote.o src/objload.o
	SPREADGINE_C:=
	CFLAGS:=-O2 -g -Iinclude -Icntools/http -Irawdraw -DHTTP_POLL_TIMEOUT=10 -DCNFG3D_USE_OGL_MAJOR $(USE_GPU)
	LDFLAGS:=-lm -lpthread $(LINK_GPU)
endif

SURVIVE:=`echo ~`/git/libsurvive
SURVIVE_CFLAGS:=-I$(SURVIVE)/include -I$(SURVIVE)/redist -DUSE_DOUBLE
SURVIVE_LDFLAGS:=$(SURVIVE)/lib/libsurvive.so -lcblas -llapacke

testspread : testspread.o $(SPREADGINE_C) $(RESOURCE_O)
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

testsurvive : testsurvive.c $(SPREADGINE_C) $(RESOURCE_O)
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS) $(SURVIVE_CFLAGS) $(SURVIVE_LDFLAGS)

testgame : testgame.c $(SPREADGINE_C) $(RESOURCE_O)
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS) $(SURVIVE_CFLAGS) $(SURVIVE_LDFLAGS)


clean :
	rm -rf testspread testsurvive testgame $(RESOURCE_O)
