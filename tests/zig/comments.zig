const std = @import("std");

pub fn main() void {
    // This is a comment
    const x = 5; // Immutable
    var y = 10; // Mutable
    
    /* This is a
    multi-line comment */
    std.debug.print("x = {}, y = {}\n", .{x, y});
}
