const builtin = @import("builtin");
const std = @import("std");

pub fn build(_: *std.Build) !void {}

pub const Package = struct {
    module: *std.Build.Module,
};

pub fn package(b: *std.Build, _: struct {}) Package {
    const module = b.addModule("oogabooga", .{
        .root_source_file = .{ .cwd_relative = thisDir() ++ "/oogabooga.zig" },
    });
    return .{ .module = module };
}

pub fn link(exe: *std.Build.Step.Compile) void {
    exe.linkLibC();
    exe.linkSystemLibrary("c");
    exe.linkSystemLibrary("Kernel32");
    exe.linkSystemLibrary("User32");
    exe.linkSystemLibrary("Gdi32");
    exe.linkSystemLibrary("shell32");
    exe.linkSystemLibrary("Avrt");
    exe.linkSystemLibrary("Winmm");
    exe.linkSystemLibrary("Dbghelp");
    exe.linkSystemLibrary("D3d11");
    exe.linkSystemLibrary("Dxguid");
    exe.linkSystemLibrary("Shlwapi");
    exe.linkSystemLibrary("Ole32");
    exe.linkSystemLibrary("Ksuser");
    exe.addIncludePath(.{ .cwd_relative = thisDir() ++ "/" });
    const c_flags = [_][]const u8{ "-std=c11", "-fno-sanitize=undefined", "-D_CRT_SECURE_NO_WARNINGS", "-Wextra", "-Wno-incompatible-library-redeclaration", "-Wno-sign-compare", "-Wno-unused-parameter", "-Wno-builtin-requires-header", "-lkernel32", "-lgdi32", "-luser32", "-lruntimeobject", "-lwinmm", "-ld3d11", "-ldxguid", "-ld3dcompiler", "-lshlwapi", "-lole32", "-lavrt", "-lksuser", "-ldbghelp", "-femit-all-decls" };
    exe.addCSourceFile(.{ .file = .{ .cwd_relative = thisDir() ++ "/oogabooga.c" }, .flags = &c_flags });
}

inline fn thisDir() []const u8 {
    return comptime std.fs.path.dirname(@src().file) orelse ".";
}
