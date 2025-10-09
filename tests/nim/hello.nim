#!/usr/bin/env nim r
# comment

#[
This is a multiline comment
which can span over several lines
]#

echo "Hello Nim World!"

var x = 2
echo x
if x > 3:
  echo "x is large"
else:
  echo "x is small"
#echo y
proc hello*(h: string): string =
  "Hello " & h & "!"

echo hello("Max")
#echo "What is you name? "
#var name: string = readLine(stdin)

#echo name
