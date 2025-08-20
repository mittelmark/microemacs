#!/usr/bin/env lua
-- this is a single line comment
print("Hello World!")
--[[ this is a multiline comment
which might end later.
]]

print("Hello World again!") --[[ again a comment ]]

--[=[ another comment
can contain [[ again and which end  here
]=]

s = [[ 
This is a multiline string!
]]
-- and here a comment

print("Hello World again 2!")

print(s)

print("The mulitline string is " .. #s .. " characters long!")

print(4 == 5)

print(4 ~= 5)

l = {1,2,3,4,5}

for i = 1, #l do
    if i % 2 == 0 then
        goto continue
    end
    print(i)
    ::continue::
end

local x = 1
print(x)

local config = {}
assert(loadfile("config.lua", "t", config))()
for option, setting in pairs(config) do
    print("key: " .. option .. " - value: " .. setting)
end

print(arg)

for i = 1, #arg do
    print(type(arg[i]))
    print(arg[i])
end

fout = io.open("hello.txt","w")
io.output(fout)
io.write("Hello 1\n")
io.write("Hello 2\n")
io.write("Hello 3\n")
io.close(fout)
do 
    local x = 0
    for line in io.lines("hello.txt") do
        x = x + 1
        print(x .. " " .. line)
    end
end
print(x)
