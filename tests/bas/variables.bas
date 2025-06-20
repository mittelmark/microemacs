'' Variable declaration examples

'' One variable per DIM statement
Dim text As String
Dim x As Double

'' More than one variable declared, different data types
Dim k As Single, factor As Double, s As String

'' More than one variable declared, all same data types
Dim As Integer mx, my, mz ,mb

'' Variable having an initializer
Dim px As Double Ptr = @x

Const upperbound = 10

'' Declare an array with indexes ranging from 0 to upperbound,
'' for a total of (upperbound + 1) indexes.
Dim array(upperbound) As Single

Type mytype
    var1 As Double
    var2 As Integer
End Type
Dim a As Byte
Dim b As Short
Dim c As Integer
Dim d As LongInt
Dim au As UByte
Dim bu As UShort
Dim cu As UInteger
Dim du As ULongInt

Dim e As Single
Dim f As Double
Dim g As Integer Ptr
Dim h As Byte Ptr
Dim s1 As String * 10   '' fixed length string
Dim s2 As String        '' variable length string
Dim s3 As ZString Ptr   '' zstring
s1 = "Hello World!"
s2 = "Hello World from FreeBASIC!"
s3 = Allocate( Len( s2 ) + 1 )
*s3 = s2
