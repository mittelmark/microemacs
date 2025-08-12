#!/usr/bin/env julia
## this a comment
#= And here comes a multiline comment
   which can be written over several lines 
   it is ended with an equal sign anfd and
   hash sign like here: =#
println("hello world")

## loop example
for i in 1:10
    if i % 3 == 0
        continue
    end
    println(i)
end 

    
