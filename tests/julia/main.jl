#!/usr/bin/env julia
function main(argv)
    for arg in argv
        println(arg)
    end
end

if abspath(PROGRAM_FILE) == abspath(@__FILE__)
    main(ARGS)
end

