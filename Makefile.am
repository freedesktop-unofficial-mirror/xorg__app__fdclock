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

EXTRA_DIST = ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
