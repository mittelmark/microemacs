{-
This is a comment
which can span multiple lines
-}

add                     :: Integer -> Integer -> Integer
add x y                 =  x + y

factorial :: Integer -> Integer
factorial 0 = 1
factorial n = do
  n * factorial (n - 1)

-- single line comment
-- here starts main
main :: IO ()
main = do
  print ("Hello Haskell World!")
  print (factorial 6)
  print (add 2 3)
