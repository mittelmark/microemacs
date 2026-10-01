#
# JASSPA MicroEmacs - www.jasspa.com
# Makefile - Portable dispatcher to the platform makefile.
#
# Plain "make" forwards to the makefile of the current platform, passing
# on goals and command line variables, e.g. "make mecb":
#
#   Linux / Cygwin / MSYS2  ->  makefiles/unixgcc.gmk    (GNU make)
#   FreeBSD                  ->  makefiles/freebsd.mak    (BSD make)
#   Darwin (macOS)           ->  makefiles/macosgcc.gmk  (GNU make)
#
# Keep this file parseable by BOTH BSD make and GNU make:
# no ifeq/.if conditionals, no $(shell) - the platform is detected
# inside the recipe with plain sh.
#

default:
	+@os=`uname`; case "$$os" in \
		FreeBSD) exec make -f makefiles/freebsd.mak ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk ;; \
		Linux|CYGWIN*|MSYS*|MINGW*) exec make -f makefiles/unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os', use one of:" ; \
		   echo "  make -f makefiles/unixgcc.gmk     (Linux, Cygwin, MSYS2)" ; \
		   echo "  make -f makefiles/freebsd.mak      (FreeBSD)" ; \
		   echo "  make -f makefiles/macosgcc.gmk   (macOS)" ; exit 1 ;; \
	esac

.DEFAULT:
	+@os=`uname`; case "$$os" in \
		FreeBSD) m=makefiles/freebsd.mak ;; \
		Darwin)  m=makefiles/macosgcc.gmk ;; \
		Linux|CYGWIN*|MSYS*|MINGW*) m=makefiles/unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac ; exec make -f "$$m" '$@'
