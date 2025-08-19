include std/io.e

integer fin = open("file.ex","r")
if fin = -1 then
   puts(1,"Could not open file!")
   abort(1)
end if
object line
integer x = 0
while 1 do
   line = gets(fin)
   
   if atom(line) then
      exit
   end if
   x = x + 1
end while
printf(1,"File has %d lines!",{x})

close(fin)
