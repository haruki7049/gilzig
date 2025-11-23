const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const mod = b.addModule("gilzig", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    const lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "gilzig",
        .root_module = mod,
    });
    b.installArtifact(lib);

    const mod_tests = b.addTest(.{
        .root_module = mod,
    });
    const run_mod_tests = b.addRunArtifact(mod_tests);

    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_mod_tests.step);

    // Create examples step
    create_examples_step(b, lib, target, optimize);
}

fn create_examples_step(
    b: *std.Build,
    lib: *std.Build.Step.Compile,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) void {
    const examples_step = b.step("examples", "Build example C programs");

    // Open and iterate through the examples directory
    // Ignore if the directory does not exist
    if (std.fs.cwd().openDir("examples", .{ .iterate = true })) |examples_dir| {
        var dir = examples_dir;
        defer dir.close();

        var it = dir.iterate();
        while (it.next() catch null) |entry| {
            // Check if it is a directory and contains main.c
            if (entry.kind == .directory) {
                const c_path = b.fmt("examples/{s}/main.c", .{entry.name});

                // Check for existence of main.c
                if (dir.access(b.fmt("{s}/main.c", .{entry.name}), .{}) catch null) |_| {
                    const mod = b.createModule(.{
                        .target = target,
                        .optimize = optimize,
                    });

                    // Add C source
                    mod.addCSourceFile(.{
                        .file = b.path(c_path),
                        .flags = &.{}, // Add C flags if needed (e.g. "-Wall")
                    });

                    // Define C executable
                    // Use directory name (e.g., how-to-use) as name
                    const exe = b.addExecutable(.{
                        .name = entry.name,
                        .root_module = mod,
                    });

                    // Link shared library
                    exe.linkLibrary(lib);

                    // Link C standard library (required for stdio.h, etc.)
                    exe.linkLibC();

                    // Install build artifact (places in zig-out/bin/)
                    const install_exe = b.addInstallArtifact(exe, .{});

                    // Add dependency to run this when examples step is executed
                    examples_step.dependOn(&install_exe.step);
                }
            }
        }
    } else |_| {
        // Do nothing if examples directory is missing
    }
}
