/* examples/how-to-use/main.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* * Event Type Definitions (based on linux/input-event-codes.h)
 * Defining them here makes this example self-contained.
 */
const int ev_syn = 0x00;
const int ev_key = 0x01;
const int ev_rel = 0x02;
const int ev_abs = 0x03;
const int ev_msc = 0x04;
const int ev_sw = 0x05;

/* * Manual declaration of library functions. */
int zctl_open(const char *path);
int zctl_poll(uint16_t *type, uint16_t *code, int32_t *value);
void zctl_close();

int main(int argc, char *argv[]) {
  /* Check if the device path is provided via command line arguments */
  if (argc < 2) {
    printf("Usage: %s <device_path>\n", argv[0]);
    printf("Example: %s /dev/input/event0\n", argv[0]);
    return 1;
  }

  const char *device_path = argv[1];

  /* * Step 1: Initialize the library */
  printf("Opening device: %s\n", device_path);
  if (zctl_open(device_path) != 0) {
    fprintf(stderr, "Error: Failed to open device '%s'.\n", device_path);
    return 1;
  }

  printf("Successfully opened. Listening for all events... (Ctrl+C to exit)\n");

  uint16_t type, code;
  int32_t value;

  /* * Step 2: Event Loop */
  while (1) {
    if (zctl_poll(&type, &code, &value) == 0) {

      switch (type) {
      case ev_syn:
        /* Synchronization event - separates packets of data */
        printf(" [SYN]  --------------------------------\n");
        break;

      case ev_key:
        /* Keys and Buttons */
        printf(" [KEY]  Code: %3d | Value: %d (%s)\n", code, value,
               (value ? "Press" : "Release"));
        break;

      case ev_rel:
        /* Relative Axis (e.g., Mouse, Scroll Wheel) */
        printf(" [REL]  Code: %3d | Value: %d\n", code, value);
        break;

      case ev_abs:
        /* Absolute Axis (e.g., Joysticks, Triggers, Touchpads) */
        printf(" [ABS]  Code: %3d | Value: %d\n", code, value);
        break;

      case ev_msc:
        /* Miscellaneous (e.g., Scan codes, timestamps) */
        printf(" [MSC]  Code: %3d | Value: %d\n", code, value);
        break;

      default:
        /* Other less common events (LED, Sound, Switch, etc.) */
        printf(" [UNK]  Type: %3d | Code: %3d | Value: %d\n", type, code,
               value);
        break;
      }
    }
    usleep(10000); /* 10ms sleep */
  }

  zctl_close();
  return 0;
}
