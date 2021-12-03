Information on the 16-bit Thunk
===============================

This directory  contains the 16-Bit  Thunking code that waits for a process to
complete.  Under  win32s  then  WinExec()  is  broken  and  does  not pass the
environment to the executed  process. We cannot use  LoadModule() as this ONLY
spawns Windows  executables. We also cannot use  CreateProcess()  on the win32
side because this is also broken !!!

So the only way  around  everything  is to create a BAT file  which  holds the
commands.  Within the BAT file we must change  directories and then invoke our
command line. This piece of code on the 16-bit side  executes the BAT file and
then waits for it to complete.

Note that I have used a rather  unorthodox  method of waiting,  simply looking
for the task that we  spawned.  I have not seen this  method  documented,  but
seems to do the job.

The only  question  mark is  whether  the win32  side  should  spawn this as a
separate thread to enable other commands to be received. At preset, while this
process in running the win32 site is hung up.

In this  directory,  do a "make  clean" to remove  everything  except the .dll
which we require. DO NOT delete the library files in this directory. These are
copied from the MSVC development  environment 1.5 in the UT samples directory.
As was the w32sut.h file.

Jon Green - 4th January 1999.
