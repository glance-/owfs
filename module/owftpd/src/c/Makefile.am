bin_PROGRAMS = owftpd
owftpd_SOURCES = file_list.c \
                 file_cd.c    \
                 ftp_command.c \
                 ftp_listener.c \
                 ftp_session.c   \
                 owftpd.c         \
                 telnet_session.c  \
                 watchdog.c         \
                 daemon_assert.c
owftpd_DEPENDENCIES = ../../../owlib/src/c/libow.la

AM_CFLAGS = -I$(srcdir)/../include \
	-I$(srcdir)/../../../owlib/src/include \
	-L../../../owlib/src/c \
	-fexceptions \
	-Wall \
	-W \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wredundant-decls \
	${PTHREAD_CFLAGS} \
	${LIBAVAHI_CFLAGS} \
	${EXTRACFLAGS}

LDADD = -low ${PTHREAD_LIBS} ${LD_EXTRALIBS} ${OSLIBS}

