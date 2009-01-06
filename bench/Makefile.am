AM_CPPFLAGS = -I$(top_builddir)/include/core -I$(top_builddir)/include/images
AM_LDFLAGS = -lpng -lpthread

bin_PROGRAMS = Bench
Bench_SOURCES = RectBench.cpp \
                SkBenchmark.cpp \
                BenchTool/main.cpp \
                $(top_builddir)/src/images/SkImageDecoder.cpp \
                $(top_builddir)/src/images/SkImageDecoder_libpng.cpp \
                $(top_builddir)/src/images/SkScaledBitmapSampler.cpp \
                $(top_builddir)/src/ports/SkGlobals_global.cpp \
                $(top_builddir)/src/ports/SkOSFile_stdio.cpp \
                $(top_builddir)/src/ports/SkThread_pthread.cpp \
                $(top_builddir)/src/ports/SkTime_Unix.cpp \
                $(top_builddir)/src/ports/SkFontHost_none.cpp


Bench_LDADD = $(top_builddir)/src/core/libskia.a 

