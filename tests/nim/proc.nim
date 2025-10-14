## public procedure names are ending with a star
proc greet*(name: string): string =
  "Hello, " & name & "!"

proc bye*(name: string): string =
  "Goodbye, " & name & "."

## private functions do not have a star after the name
proc communicate(greeting: proc(x: string): string, name: string) =
  echo greeting(name)

## public function exported from module
## functions have no side effects
func square* (x: int): int =
  ## echo "Hi" ## would fail
  return x * x
  
when isMainModule:
    communicate(greet, "John")
    communicate(bye, "Mary")
    echo square(6)
