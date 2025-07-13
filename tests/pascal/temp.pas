
{****************************************************************************
 *
 * Description : Hello World Application
 *
 * Notes : Demonstration for MicroEmacs
 *
 * History :
 *	
 ****************************************************************************
 *
 * Copyright (c) 2025 Detlef Groth.
 * 
 ****************************************************************************}

Program HelloWorld;

Uses crt;

Var 
  a, b, ret: Integer ;

Type Books = Record
  title: packed array [1..50] Of char;
  author: packed array [1..50] Of char;
  subject: packed array [1..100] Of char;
  book_id: integer;
End;

Function add(a, b: Integer ): Integer ;

Var 
  result: Integer;
Begin
  result := a+b;
  add := result;
End;
Procedure hello( );
Begin
  writeln('Hello, World!');
  readkey;
End;
(* Here the main program block starts *)
Begin {Main}
  a := 10;
  b := 20;
  ret := add(a,b);
  writeln('Addition result is: ', ret);
  hello();
End.
