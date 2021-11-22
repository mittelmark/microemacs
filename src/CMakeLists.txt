project(me)
option(ENABLE_GUI "Build GUI" ON)
set (SRCS abbrev.c
          basic.c
          bind.c
          buffer.c
          crypt.c
          dirlist.c
          termio.c
          display.c
          eval.c
          exec.c
          file.c
          fileio.c
          frame.c
          hilight.c
          history.c
          input.c
          isearch.c
          key.c
          line.c
          macro.c
          main.c
          narrow.c
          next.c
          osd.c
          print.c
          random.c
          regex.c
          region.c
          registry.c
          search.c
          spawn.c
          spell.c
          tag.c
          time.c
          undo.c
          window.c
          word.c)

set (WIN32_SRCS winterm.c
                winprint.c
                me.rc)
set (VTE_SRCS unixterm.c)
if (WIN32)
    list(APPEND SRCS ${WIN32_SRCS})
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
endif()
if (APPLE)
    add_definitions(-D_DARWIN)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-D_LINUX)
    add_definitions(-D_LINUX26)
endif()
if (UNIX)
    add_definitions(-D_LARGEFILE_SOURCE)
    add_definitions(-D_FILE_OFFSET_BITS=64)
    list(APPEND SRCS ${VTE_SRCS})
endif()
if (ENABLE_GUI)
    add_definitions(-D_ME_WINDOW)
endif()
add_definitions(-D_ME_CONSOLE)
add_executable(me ${SRCS})
if (UNIX)
    find_library(TERMCAP_LIBRARY NAMES tinfo termcap ncursesw ncurses cursesw curses)
    target_link_libraries(me ${TERMCAP_LIBRARY})
endif()
