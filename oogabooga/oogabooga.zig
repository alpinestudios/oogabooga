//usingnamespace @cImport(@cInclude("oogabooga.h"));
pub extern fn hello_world() void;

const c = @cImport(@cInclude("./oogabooga.h"));
const std = @import("std");

pub fn test_f() void {
    c.hello_world();
    std.debug.print(" {s} \n", .{"codebase"});
}
