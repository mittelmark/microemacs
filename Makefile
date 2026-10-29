#
# JASSPA MicroEmacs - www.jasspa.com
# Makefile - Portable dispatcher to the platform makefile.
#
# Plain "make" forwards to the makefile of the current platform, passing
# on goals and command line variables, e.g. "make mecb":
#
#   Linux / Cygwin           ->  makefiles/unixgcc.gmk      (GNU make)
#   MSYS2                    ->  Unix-path console plus native Windows builds
#   MinGW                    ->  makefiles/winmingwgcc.gmk  (GNU make)
#   FreeBSD                  ->  makefiles/freebsd.mak      (BSD make)
#   Darwin (macOS)           ->  makefiles/macosgcc.gmk     (GNU make)
#
# Keep this file parseable by BOTH BSD make and GNU make:
# no ifeq/.if conditionals, no $(shell) - the platform is detected
# inside the recipe with plain sh.
#

default:
	+@os=`uname`; case "$$os" in \
		FreeBSD) exec make -f makefiles/freebsd.mak ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk ;; \
		MSYS*|MINGW*) exec make -f makefiles/winmingwgcc.gmk ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os', use one of:" ; \
		   echo "  make -f makefiles/unixgcc.gmk     (Linux, Cygwin)" ; \
		   echo "  make -f makefiles/winmingwgcc.gmk (MSYS2)" ; \
		   echo "  make -f makefiles/freebsd.mak      (FreeBSD)" ; \
		   echo "  make -f makefiles/macosgcc.gmk   (macOS)" ; exit 1 ;; \
	esac

# MSYS and UCRT64 shells can build the mintty console with /usr/bin/gcc and
# the native Windows variants with the compiler selected by winmingwgcc.gmk.
# MINGW shells build only the native Windows variants.
mec:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS|MSYS*|UCRT64) $(MAKE) -f makefiles/unixgcc.gmk CC=/usr/bin/gcc mec && \
		       $(MAKE) -f makefiles/winmingwgcc.gmk mec BDIST=mingw64 ;; \
		MINGW*) $(MAKE) -f makefiles/winmingwgcc.gmk mec BDIST=mingw64 ;; \
		FreeBSD) exec make -f makefiles/freebsd.mak "$@" ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk "$@" ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk "$@" ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac

mew:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS*|MINGW*) exec $(MAKE) -f makefiles/winmingwgcc.gmk mew BDIST=mingw64 ;; \
		UCRT64) exec $(MAKE) -f makefiles/winmingwgcc.gmk mew BDIST=mingw64 ;; \
		FreeBSD) exec make -f makefiles/freebsd.mak "$@" ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk "$@" ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk "$@" ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac

mecb:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS|MSYS*|UCRT64) $(MAKE) mecb-msys && \
		       $(MAKE) -f makefiles/winmingwgcc.gmk mecb BDIST=mingw64 ;; \
		MINGW*) $(MAKE) -f makefiles/winmingwgcc.gmk mecb BDIST=mingw64 ;; \
		FreeBSD) exec make -f makefiles/freebsd.mak "$@" ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk "$@" ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk "$@" ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac

mec-msys:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS|MSYS*|UCRT64) exec $(MAKE) -f makefiles/unixgcc.gmk CC=/usr/bin/gcc mec ;; \
		*) echo "make: mec-msys is only supported on MSYS2" >&2 ; exit 1 ;; \
	esac

mecb-msys:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS|MSYS*|UCRT64) exec $(MAKE) -f makefiles/unixgcc.gmk CC=/usr/bin/gcc mecb ;; \
		*) echo "make: mecb-msys is only supported on MSYS2" >&2 ; exit 1 ;; \
	esac

mewb:
	+@platform=$${MSYSTEM:-$$(uname -s)}; case "$$platform" in \
		MSYS|MSYS*|UCRT64|MINGW*) exec $(MAKE) -f makefiles/winmingwgcc.gmk mewb BDIST=mingw64 ;; \
		FreeBSD) exec make -f makefiles/freebsd.mak "$@" ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk "$@" ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk "$@" ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac

release:
	+@platform=$${MSYSTEM:-$$(uname -s)}; app='$(app)'; app=$${app:-mecb}; \
	case "$$platform" in \
		MSYS|MSYS*|UCRT64) \
			case "$$app" in \
				mecb) $(MAKE) -f makefiles/unixgcc.gmk CC=/usr/bin/gcc release app=mecb && \
				      $(MAKE) -f makefiles/winmingwgcc.gmk release app=mecb BDIST=mingw64 ;; \
				*) exec $(MAKE) -f makefiles/winmingwgcc.gmk release app="$$app" BDIST=mingw64 ;; \
			esac ;; \
		MINGW*) exec $(MAKE) -f makefiles/winmingwgcc.gmk release app="$$app" BDIST=mingw64 ;; \
		FreeBSD) exec make -f makefiles/freebsd.mak release app="$$app" ;; \
		Darwin)  exec make -f makefiles/macosgcc.gmk release app="$$app" ;; \
		Linux|CYGWIN*) exec make -f makefiles/unixgcc.gmk release app="$$app" ;; \
		*) echo "make: unsupported platform '$$platform'" >&2 ; exit 1 ;; \
	esac

.DEFAULT:
	+@os=`uname`; case "$$os" in \
		FreeBSD) m=makefiles/freebsd.mak ;; \
		Darwin)  m=makefiles/macosgcc.gmk ;; \
		MSYS*|MINGW*) m=makefiles/winmingwgcc.gmk ;; \
		Linux|CYGWIN*) m=makefiles/unixgcc.gmk ;; \
		*) echo "make: unsupported platform '$$os'" >&2 ; exit 1 ;; \
	esac ; exec make -f "$$m" '$@'
