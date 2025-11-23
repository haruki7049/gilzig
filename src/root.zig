const std = @import("std");
const fs = std.fs;
const os = std.os;

// Linux input_event structure (from linux/input.h)
// Alignment matches C ABI on Linux (usually).
const InputEvent = extern struct {
    time: extern struct {
        sec: c_long,
        usec: c_long,
    },
    type: u16,
    code: u16,
    value: i32,
};

// Global state to hold the file descriptor
var device_file: ?fs.File = null;

// Error codes exposed to C
const ZCTL_OK: i32 = 0;
const ZCTL_ERR_OPEN: i32 = -1;
const ZCTL_ERR_READ: i32 = -2;
const ZCTL_NO_DATA: i32 = -3;

/// Initialize the library by opening a device path.
/// path: Null-terminated C string (e.g., "/dev/input/event0")
export fn zctl_open(path: [*:0]const u8) i32 {
    const slice = std.mem.span(path);

    // Open file in non-blocking mode
    const file = fs.openFileAbsolute(slice, .{ .mode = .read_only }) catch {
        return ZCTL_ERR_OPEN;
    };

    device_file = file;
    return ZCTL_OK;
}

/// Poll for the next input event.
/// type, code, value: Pointers to store result.
/// Returns 0 on success, negative on error/empty.
export fn zctl_poll(ev_type: *u16, ev_code: *u16, ev_value: *i32) i32 {
    if (device_file) |file| {
        var event: InputEvent = undefined;

        // Read exactly one struct size
        const bytes_read = file.read(std.mem.asBytes(&event)) catch |err| {
            // Normally we would check for WouldBlock, assuming non-blocking I/O logic here
            // For simplicity, treating generic errors as read error.
            _ = err;
            return ZCTL_ERR_READ;
        };

        if (bytes_read != @sizeOf(InputEvent)) {
            return ZCTL_NO_DATA;
        }

        ev_type.* = event.type;
        ev_code.* = event.code;
        ev_value.* = event.value;

        return ZCTL_OK;
    }
    return ZCTL_ERR_OPEN;
}

/// Close the device.
export fn zctl_close() void {
    if (device_file) |file| {
        file.close();
        device_file = null;
    }
}
