const std = @import("std");

// TODO: test code

const Color = enum {
    Red,
    Green,
    Blue,
};

pub fn testfunc() void {
    std.debug.print("Hello Test! Color is {}!",.{Color.Red}) ;
}
const Person = struct {
    name: []const u8,
    age: u32,
    favorite_color: Color,

    fn introduce(self: Person) void {
        std.debug.print("Hi, I'm {s}, I'm {} years old, and my favorite color is {}\n", .{
            self.name, self.age, self.favorite_color,
        });
    }
};

pub fn main() void {
    const bob = Person{
        .name = "Bob",
        .age = 30,
        .favorite_color = Color.Blue,
    };
    bob.introduce();
    testfunc();
}
test "this is a test" {
    std.debug.print("Hello",.{}) ;
}
