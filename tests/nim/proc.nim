proc greet(name: string): string =
  "Hello, " & name & "!"

proc bye(name: string): string =
  "Goodbye, " & name & "."

proc communicate(greeting: proc(x: string): string, name: string) =
  echo greeting(name)

when isMainModule:
    communicate(greet, "John")
    communicate(bye, "Mary")
