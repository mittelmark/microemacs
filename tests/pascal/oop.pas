{****************************************************************************
 *  Created By    : Detlef Groth
 *  Created       : Sun Jul 13 09:24:58 2025
 *  Last Modified : <250713.0928>
 *
 *  Description	  : Example of OOP in Pascal for MicroEmacs
 *
 ****************************************************************************}

Program exObjects;

Type Rectangle = Object
  Private 
    length, width: integer;
  Public 
    Procedure setlength(l: integer);
    Function getlength(): integer;
    Procedure setwidth(w: integer);
    Function getwidth(): integer;
    Procedure draw;
End;

Var 
  r1: Rectangle;
  pr1: ^Rectangle;
Procedure Rectangle.setlength(l: integer);
Begin
  length := l;
End;
Procedure Rectangle.setwidth(w: integer);
Begin
  width := w;
End;
Function Rectangle.getlength(): integer;
Begin
  getlength := length;
End;
Function Rectangle.getwidth(): integer;
Begin
  getwidth := width;
End;
Procedure Rectangle.draw;

Var 
  i, j: integer;
Begin
  For i:= 1 To length Do
    Begin
      For j:= 1 To width Do
        write(' * ');
      writeln;
    End;
End;

Begin
  r1.setlength(3);
  r1.setwidth(7);
  writeln('Draw a rectangle:', r1.getlength(), ' by ' , r1.getwidth());
  r1.draw;
  new(pr1);
  pr1^.setlength(5);
  pr1^.setwidth(4);
  writeln('Draw a rectangle:', pr1^.getlength(), ' by ' ,pr1^.getwidth());
  pr1^.draw;
  dispose(pr1);
End.
