function AddNumbers( a as integer, b as integer ) as integer
  return a + b
end function

sub hello( )
  print "hello"
end sub

declare sub myprint( num as integer )


sub myprint( num as integer )
  print num
end sub

' just use to follow some conventions
sub main ()
  hello( )
  print AddNumbers( 1, 1 )
  myprint 6
end sub

'Code outside any procedures is the main part of the program
main( )
