# This file was generated by "make depend".
#

src/main.o: src/main.cc $(LIBAUDIO_ROOT)include/AudioDevice.h $(LIBAUDIO_ROOT)include/AudioSource.h $(LIBCOMMON_ROOT)include/common/c++/handle.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/c++/stream.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/logger.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/refcnt.h $(LIBPOLLSTER_ROOT)include/pollster/filter.h $(LIBPOLLSTER_ROOT)include/pollster/pollster.h $(LIBPOLLSTER_ROOT)include/pollster/sockapi.h $(LIBPOLLSTER_ROOT)include/pollster/ssl.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
