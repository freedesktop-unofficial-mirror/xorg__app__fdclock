INCLUDES = @FDCLOCK_CFLAGS@

COMMON_SRCS = fdlogo.c fdface.c

fdclock_SOURCES = \
	fdclock.c fdhand.c findargb.c \
	$(COMMON_SRCS)

fdfacepng_SOURCES = \
	fdfacepng.c \
	$(COMMON_SRCS)

bin_PROGRAMS = fdclock fdfacepng

fdclock_LDADD = @FDCLOCK_LIBS@

fdfacepng_LDADD = @FDCLOCK_LIBS@ @PNG_LIBS@
