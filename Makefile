MAKEFILES_ROOT=submodules/makefiles/
LIBCOMMON_ROOT=submodules/common/
LIBAUDIO_ROOT=submodules/audio/
LIBPOLLSTER_ROOT=submodules/pollster/

.PHONY: all
all: lirc-volume

include ${LIBAUDIO_ROOT}Makefile.inc
include ${LIBPOLLSTER_ROOT}Makefile.inc

LDFLAGS += $(CXXLIBS)

LDFLAGS+=-L$(LIBAUDIO_ROOT) -laudio
LDFLAGS+=-L$(LIBPOLLSTER_ROOT) -lpollster
LDFLAGS+=-L$(LIBCOMMON_ROOT) -lcommon

SRCFILES += \
   src/main.cc

OBJS += $(shell $(SRC2OBJ) $(SRCFILES))

lirc-volume: ${LIBCOMMON} ${LIBAUDIO} ${LIBPOLLSTER} ${OBJS}
	${CXX} -o $@ ${OBJS} ${LDFLAGS}

.PHONY: clean
clean:
	rm -f lirc-volume ${OBJS}
	rm -f ${LIBCOMMON} ${LIBCOMMON_OBJS}
	rm -f ${LIBAUDIO} ${LIBAUDIO_OBJS}
	rm -f ${LIBPOLLSTER} ${LIBPOLLSTER_OBJS}

INCLUDES+= \
   -I$(LIBAUDIO_ROOT)include \
   -I$(LIBCOMMON_ROOT)include \
   -I$(LIBPOLLSTER_ROOT)include
CFLAGS+=$(INCLUDES)
CXXFLAGS+=$(INCLUDES)

LIB_ROOTS= \
   LIBAUDIO_ROOT=$(LIBAUDIO_ROOT) \
   LIBCOMMON_ROOT=$(LIBCOMMON_ROOT) \
   LIBPOLLSTER_ROOT=$(LIBPOLLSTER_ROOT) \
   MAKEFILES_ROOT=$(MAKEFILES_ROOT)

.PHONY: depend
depend:
	env CFLAGS="$(CFLAGS)" $(LIB_ROOTS) \
	$(DEPEND) src/*.cc > depend.mk

-include depend.mk
