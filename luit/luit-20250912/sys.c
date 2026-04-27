/* $XTermId: sys.c,v 1.61 2025/09/12 08:20:14 tom Exp $ */

/*
Copyright 2010-2024,2025 by Thomas E. Dickey
Copyright (c) 2001 by Juliusz Chroboczek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <luit.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

#ifdef HAVE_SETGROUPS
#include <grp.h>
#endif

#ifdef HAVE_WORKING_POLL
#ifdef HAVE_POLL_H
#include <poll.h>
#else
#include <sys/poll.h>
#endif
#undef HAVE_SELECT
#endif

#ifdef HAVE_SELECT
#if !(defined(_MINIX) || defined(__BEOS__))
#define HAVE_WORKING_SELECT 1
#endif
#endif

#ifdef HAVE_WORKING_SELECT
#if defined(HAVE_SYS_SELECT_H) && defined(HAVE_SYS_TIME_SELECT)
#include <sys/select.h>
#endif
#endif

#ifdef HAVE_PTY_H
#include <pty.h>
#endif

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_OPENPTY
#if defined(HAVE_UTIL_H)
#include <util.h>
#elif defined(HAVE_LIBUTIL_H)
#include <libutil.h>
#elif defined(HAVE_PTY_H)
#include <pty.h>
#endif
#endif /* HAVE_OPENPTY */

#include <sys.h>
#include <trace.h>

#ifdef USE_IGNORE_RC
int ignore_unused;
#endif

#if defined(HAVE_OPENPTY)
static int opened_tty = -1;
#endif

#if defined(I_FIND) && defined(I_PUSH)
#define PUSH_FAILS(fd,name) ioctl(fd, I_FIND, name) == 0 \
			 && ioctl(fd, I_PUSH, name) < 0
#else
#define PUSH_FAILS(fd,name) ioctl(fd, I_PUSH, name) < 0
#endif

static int saved_tio_valid = 0;
static struct termios saved_tio;

int
waitForOutput(int fd)
{
    int ret = 0;

#if defined(HAVE_WORKING_POLL)
    struct pollfd pfd[1];
    int rc;

    pfd[0].fd = fd;
    pfd[0].events = POLLOUT;
    pfd[0].revents = 0;

    rc = poll(pfd, (nfds_t) 1, -1);
    if (rc < 0) {
	ret = -1;
    } else if (pfd[0].revents & (POLLOUT | POLLERR | POLLHUP)) {
	ret = IO_CanWrite;
    } else if (pfd[0].revents & (POLLNVAL)) {
	ret = IO_Closed;
    }
#elif defined(HAVE_WORKING_SELECT)
    fd_set fds;
    int rc;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    rc = select(FD_SETSIZE, NULL, &fds, NULL, NULL);
    if (rc < 0) {
	ret = -1;
	if (errno == EBADF)
	    ret = IO_Closed;
    } else if (FD_ISSET(fd, &fds)) {
	ret = IO_CanWrite;
    }
#else
    ret = IO_CanWrite;
#endif

    return ret;
}

int
waitForInput(int fd1, int fd2)
{
    int ret = 0;

#if defined(HAVE_WORKING_POLL)
    struct pollfd pfd[2];
    int rc;

    pfd[0].fd = fd1;
    pfd[1].fd = fd2;
    pfd[0].events = pfd[1].events = POLLIN;
    pfd[0].revents = pfd[1].revents = 0;

    rc = poll(pfd, (nfds_t) 2, -1);
    if (rc < 0) {
	ret = -1;
    } else {
	if (pfd[0].revents & (POLLIN | POLLERR | POLLHUP))
	    ret |= IO_CanRead;
	if (pfd[1].revents & (POLLIN | POLLERR | POLLHUP))
	    ret |= IO_CanWrite;
	if (pfd[0].revents & (POLLNVAL))
	    ret |= IO_Closed;
	if (pfd[1].revents & (POLLNVAL))
	    ret |= IO_Closed;
    }

#elif defined(HAVE_WORKING_SELECT)
    fd_set fds;
    int rc;

    FD_ZERO(&fds);
    FD_SET(fd1, &fds);
    FD_SET(fd2, &fds);
    rc = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
    if (rc < 0) {
	ret = -1;
	if (errno == EBADF)
	    ret = IO_Closed;
    } else {
	if (FD_ISSET(fd1, &fds))
	    ret |= IO_CanRead;
	if (FD_ISSET(fd2, &fds))
	    ret |= IO_CanWrite;
    }
#else
    ret = (IO_CanRead | IO_CanWrite);
#endif

    return ret;
}

int
setWindowSize(int sfd, int dfd)
{
#ifdef TIOCGWINSZ
    int rc;
    struct winsize ws;
    TRACE(("setWindowSize sfd %d, dfd %d\n", sfd, dfd));
    rc = ioctl(sfd, TIOCGWINSZ, (char *) &ws);
    if (rc < 0)
	return -1;
    TRACE(("...got %dx%d\n", ws.ws_row, ws.ws_col));
    rc = ioctl(dfd, TIOCSWINSZ, (char *) &ws);
    if (rc < 0)
	return -1;
#endif
    return 0;
}

int
installHandler(int signum, void (*handler) (int))
{
    struct sigaction sa;
    sigset_t ss;
    int rc;

    sigemptyset(&ss);

    sa.sa_handler = handler;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    rc = sigaction(signum, &sa, NULL);
    return rc;
}

int
copyTermios(int sfd, int dfd)
{
    struct termios tio;
    int rc;

#if !defined(HAVE_GRANTPT) || defined(HAVE_GRANTPT_TCSETATTR)
    TRACE(("copyTermios(sfd %d, dfd %d)\n", sfd, dfd));
    rc = tcgetattr(sfd, &tio);
    if (rc < 0) {
	TRACE_ERR("copyTermios - tcgetattr");
    } else {
	rc = tcsetattr(dfd, TCSAFLUSH, &tio);
	if (rc < 0) {
	    TRACE_ERR("copyTermios - tcsetattr");
	}
    }
#else
    (void) sfd;
    (void) dfd;
    (void) tio;
    rc = 0;
#endif

    return rc;
}

static int
saveTermios(int sfd)
{
    int rc;
    rc = tcgetattr(sfd, &saved_tio);
    if (rc >= 0)
	saved_tio_valid = 1;
    return rc;
}

int
restoreTermios(int sfd)
{
    if (!saved_tio_valid)
	return -1;
    return tcsetattr(sfd, TCSAFLUSH, &saved_tio);
}

int
setRawTermios(int sfd)
{
    struct termios tio;
    int rc;

    if (!saved_tio_valid)
	saveTermios(sfd);
    rc = tcgetattr(sfd, &tio);
    if (rc < 0)
	return rc;
    tio.c_lflag &= (unsigned) ~(ECHO | ICANON | IEXTEN | ISIG);
    tio.c_iflag &= (unsigned) ~(ICRNL | IXOFF | IXON | ISTRIP);
#ifdef ONLCR
    tio.c_oflag &= (unsigned) ~ONLCR;
#endif
#ifdef OCRNL
    tio.c_oflag &= (unsigned) ~OCRNL;
#endif
#ifdef ONOCR
    tio.c_oflag &= (unsigned) ~ONOCR;
#endif

#ifdef VMIN
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
#endif
    rc = tcsetattr(sfd, TCSAFLUSH, &tio);
    if (rc < 0)
	return rc;
    return 0;
}

char *
my_basename(char *path)
{
    char *p;

    p = strrchr(path, '/');
    if (!p)
	p = path;
    else
	p++;
    return p;
}

static int
fix_pty_perms(char *line)
{
    int rc;
    struct stat s;

    rc = stat(line, &s);
    if (rc < 0)
	return -1;

    TRACE(("fix_pty_perms sees %s\n", NonNull(line)));
    TRACE(("..pty owned by %ld/%ld\n", (long) s.st_uid, (long) s.st_gid));
    TRACE(("...but you are %ld/%ld\n", (long) getuid(), (long) getgid()));

    if (s.st_uid != getuid() || s.st_gid != getgid()) {
	rc = chown(line, getuid(), getgid());
	if (rc < 0) {
	    Warning("could not change ownership of tty -- "
		    "pty is insecure!\n");
	    return 0;
	}
    }
    if ((s.st_mode & 0777) != (S_IRUSR | S_IWUSR | S_IWGRP)) {
	rc = chmod(line, S_IRUSR | S_IWUSR | S_IWGRP);
	if (rc < 0) {
	    Warning("could not change permissions of tty -- "
		    "pty is insecure!\n");
	    return 0;
	}
    }
    return 1;
}

/*
 * APUE2 notes that some systems lack posix_openpt(), which should be provided
 * when grantpt() is available.
 */
#ifndef HAVE_POSIX_OPENPT
#undef posix_openpt
#define posix_openpt(mode) open("/dev/ptmx", mode)
#endif

int
allocatePty(int *pty_return, char **line_return)
{
    char name[12], *line = NULL;
    int pty = -1;
    const char *name1 = "pqrstuvwxyzPQRST";
    char buffer[80];
    char *name2 = strcpy(buffer, "0123456789abcdefghijklmnopqrstuv");
    const char *p1;
    char *p2;

#if defined(HAVE_GRANTPT) && defined(HAVE_WORKING_GRANTPT)
    int rc;

    TRACE(("allocatePty (posix)\n"));
    /*
     * hmm - xterm does open(/dev/ptmx), grant, unlock, open(ptsname)
     * The second open is in child().
     * xterm puts the streams setup after the second open().
     */
    pty = posix_openpt(O_RDWR);
    if (pty < 0) {
	TRACE_ERR("allocatePty - posix_openpt");
	goto bsd;
    }

    rc = grantpt(pty);
    if (rc < 0) {
	TRACE_ERR("allocatePty - grantpt");
	close(pty);
	goto bsd;
    }

    rc = unlockpt(pty);
    if (rc < 0) {
	TRACE_ERR("allocatePty - unlockpt");
	close(pty);
	goto bsd;
    }

    /* if ptsname follows the streams setup, it returns null */
    line = strmalloc(ptsname(pty));
    if (!line) {
	TRACE_ERR("allocatePty - ptsname");
	close(pty);
	goto bsd;
    }

    TRACE(("ptsname '%s'\n", NonNull(line)));
    fix_pty_perms(line);

    *pty_return = pty;
    *line_return = line;
    return 0;

  bsd:
#elif defined(HAVE_OPENPTY)
    int rc;
    char ttydev[80];		/* OpenBSD says at least 16 bytes */

    TRACE(("allocatePty (openpty)\n"));
    rc = openpty(&pty, &opened_tty, ttydev, NULL, NULL);
    if (rc < 0) {
	close(pty);
	goto bsd;
    }
    line = strmalloc(ttydev);
    if (!line) {
	close(pty);
	goto bsd;
    }

    fix_pty_perms(line);

    *pty_return = pty;
    *line_return = line;
    return 0;

  bsd:
#endif /* HAVE_GRANTPT, etc */

    TRACE(("allocatePty (bsd)\n"));
    strcpy(name, "/dev/pty??");
    for (p1 = name1; *p1; p1++) {
	name[8] = *p1;
	for (p2 = name2; *p2; p2++) {
	    name[9] = *p2;
	    pty = open(name, O_RDWR);
	    if (pty >= 0)
		goto found;
	    /* Systems derived from 4.4BSD differ in their pty names,
	       so ENOENT doesn't necessarily imply we're done. */
	    continue;
	}
    }

    goto bail;

  found:
    if ((line = strmalloc(name)) != NULL) {
	line[5] = 't';
	fix_pty_perms(line);
	*pty_return = pty;
	*line_return = line;
	return 0;
    }

    close(pty);

  bail:
    return -1;
}

int
openTty(char *line)
{
    int tty = -1;

    TRACE(("openTty(%s)\n", NonNull(line)));
    if (line == NULL)
	ExitFailure();

    tty = open(line, O_RDWR
#if defined(TIOCSCTTY) && defined(O_NOCTTY)
    /*
     * Do not make this our controlling terminal, yet just in case it fails
     * in some intermediate state.  But do not add this flag if we haven't
     * the corresponding ioctl.
     */
	       | O_NOCTTY
#endif
	);

    if (tty < 0)
	goto bail;

#if defined(HAVE_OPENPTY)
    if (opened_tty >= 0) {
	close(opened_tty);
	opened_tty = -1;
    }
#endif

#ifdef TIOCSCTTY
    /*
     * Now that we've successfully opened the terminal, make it the controlling
     * terminal.  This call works only if the process does not already have a
     * controlling terminal.
     *
     * Cygwin as of 2009/10/12 lacks this call, but has O_NOCTTY.
     */
    if (ioctl(tty, TIOCSCTTY, (char *) 0) < 0) {
	TRACE_ERR("openTty - ioctl(TIOCSCTTY)");
	goto bail;
    }
#endif

    /* HPUX and Solaris */
#ifdef HAVE_GRANTPT_PTEM
    TRACE(("...setup stream\n"));
    if (PUSH_FAILS(tty, "ptem")) {
	TRACE_ERR("openTty - push ptem");
	goto bail;
    }
#endif

#ifdef HAVE_GRANTPT_LDTERM
    if (PUSH_FAILS(tty, "ldterm")) {
	TRACE_ERR("openTty - push ldterm");
	goto bail;
    }
#endif

    /* Solaris */
#ifdef HAVE_GRANTPT_TTCOMPAT
    if (PUSH_FAILS(tty, "ttcompat")) {
	TRACE_ERR("openTty - push ttcompat");
	goto bail;
    }
#endif

    return tty;

  bail:
    if (tty >= 0)
	close(tty);
    return -1;
}

/* Post-4.4 BSD systems have POSIX semantics (_POSIX_SAVED_IDS
   or not, depending on the version).  4.3BSD and Minix do not have
   saved IDs at all, so there's no issue. */
int
droppriv(void)
{
    int rc;
#if (defined(BSD) && !defined(_POSIX_SAVED_IDS)) || defined(_MINIX)
    rc = setuid(getuid());
    if (rc >= 0) {
	rc = setgid(getgid());
    }
#elif defined(_POSIX_SAVED_IDS)
    uid_t uid = getuid();
    uid_t euid = geteuid();
    gid_t gid = getgid();
    gid_t egid = getegid();

    if ((uid != euid || gid != egid) && euid != 0) {
	errno = ENOSYS;
	rc = -1;
    } else {
#ifdef HAVE_SETGROUPS
	setgroups(0, NULL);
#endif
	rc = setgid(gid);
	if (rc >= 0)
	    rc = setuid(uid);
    }
#else
    uid_t uid = getuid();
    uid_t euid = geteuid();
    gid_t gid = getgid();
    gid_t egid = getegid();

    if (uid != euid || gid != egid) {
	errno = ENOSYS;
	rc = -1;
    } else {
	rc = 0;
    }
#endif
    return rc;
}

#ifndef HAVE_STRCASECMP
int
my_strcasecmp(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
	int p = toupper(*a++);
	int q = toupper(*b++);
	int r;
	if ((r = p - q) != 0)
	    return r;
    }
    return *a - *b;
}
#endif

char *
strmalloc(const char *value)
{
    char *result = NULL;

    if (value != NULL) {
#ifdef HAVE_STRDUP
	result = strdup(value);
#else
	result = malloc(strlen(value) + 1);
	if (result != 0)
	    strcpy(result, value);
#endif
    }
    return result;
}

#ifdef NO_LEAKS
void
ExitProgram(int code)
{
    TRACE(("ExitProgram %d\n", code));
    luit_leaks();
    iso2022_leaks();
    charset_leaks();
    fontenc_leaks();
#ifdef USE_ICONV
    luitconv_leaks();
#endif
    exit(code);
}
#endif
