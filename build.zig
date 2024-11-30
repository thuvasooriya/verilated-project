const std = @import("std");
const fs = std.fs;
const print = std.debug.print;
const run = std.ChildProcess.exec;

pub fn build(b: *std.Build) void {
    const module_name = "alu";

    // Verilate step
    const verilate_step = b.step("verilate", "run verilator to generate c++ code");
    const verilate_cmd = b.addSystemCommand(&[_][]const u8{
        "verilator",
        "-CFLAGS",
        "-std=c++14",
        "-Wall",
        "--trace",
        "-cc",
        b.fmt("{s}.sv", .{module_name}),
        "--exe",
        b.fmt("{s}_tb.cpp", .{module_name}),
    });
    verilate_step.dependOn(&verilate_cmd.step);

    // Add a command to touch .stamp.verilate
    const touch_stamp = b.addSystemCommand(&[_][]const u8{
        "touch",
        ".stamp.verilate",
    });
    touch_stamp.step.dependOn(&verilate_cmd.step);
    verilate_step.dependOn(&touch_stamp.step);

    // Build step
    const build_step = b.step("build", "Build the simulation executable");
    const build_cmd = b.addSystemCommand(&[_][]const u8{
        "make",
        "-C",
        "obj_dir",
        "-f",
        b.fmt("V{s}.mk", .{module_name}),
        b.fmt("V{s}", .{module_name}),
    });
    build_step.dependOn(verilate_step);
    build_step.dependOn(&build_cmd.step);

    // Simulate step
    const sim_step = b.step("sim", "Run the simulation and generate waveform");
    const sim_cmd = b.addSystemCommand(&[_][]const u8{
        b.fmt("./obj_dir/V{s}", .{module_name}),
    });
    sim_step.dependOn(build_step);
    sim_step.dependOn(&sim_cmd.step);

    // Waves step
    const waves_step = b.step("waves", "View waveform using GTKWave");
    const waves_cmd = b.addSystemCommand(&[_][]const u8{
        "gtkwave",
        "-6",
        "waveform.vcd",
    });
    waves_step.dependOn(sim_step);
    waves_step.dependOn(&waves_cmd.step);

    // Lint step
    const lint_step = b.step("lint", "Run Verilator in lint-only mode");
    const lint_cmd = b.addSystemCommand(&[_][]const u8{
        "verilator",
        "--lint-only",
        b.fmt("{s}.sv", .{module_name}),
    });
    lint_step.dependOn(&lint_cmd.step);

    // Clean step
    const clean_step = b.step("clean", "Remove all generated files and directories");
    const clean_cmd = b.addSystemCommand(&[_][]const u8{
        "rm",
        "-rf",
        ".stamp.*",
        "obj_dir",
        "waveform.vcd",
    });
    clean_step.dependOn(&clean_cmd.step);

    // Set default step
    b.default_step = sim_step;
}
