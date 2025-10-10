#import std/macros

macro myMacro(arg: static[int]): untyped =
  echo arg # just an int (7), not `NimNode`
  

myMacro(1 + 2 * 3)
