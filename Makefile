#
# JASSPA MicroEmacs - www.jasspa.com
# Makefile - Portable dispatcher to the platform makefile.
#
# Plain "make" forwards to the makefile of the current platform, passing
# on goals and command line variables, e.g. "make mecb":
#
#   Linux / Cygwin / MSYS2  ->  unixgcc.gmk     (GNU make)
#   FreeBSD                  ->  freebsd.mak     (BSD make)
#   Darwin (macOS)           ->  macos32gcc.gmk  (GNU make)
#
# Keep this file parseable by BOTH BSD make and GNU make:
# no ifeq/.if conditionals, no $(shell) - the platform is detected
# inside the recipe with plain sh.
#

default:
	+@os=`uname`; case "$$os" in \
		FreeBSD) exec make -f freebsd.mak ;; \
		Darwin)  exec make -f macos32gcc.gmk ;; \
		Linux|CYGWIN*|MSYS*|MINGW*) exec make -f unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os', use one of:" ; \
		   echo "  make -f unixgcc.gmk     (Linux, Cygwin, MSYS2)" ; \
		   echo "  make -f freebsd.mak      (FreeBSD)" ; \
		   echo "  make -f macos32gcc.gmk   (macOS)" ; exit 1 ;; \
	esac

.DEFAULT:
	+@os=`uname`; case "$$os" in \
		FreeBSD) m=freebsd.mak ;; \
		Darwin)  m=macos32gcc.gmk ;; \
		Linux|CYGWIN*|MSYS*|MINGW*) m=unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac ; exec make -f "$$m" '$@'
