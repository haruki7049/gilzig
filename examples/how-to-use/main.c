/* examples/how-to-use/main.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* * Manual declaration of library functions.
 * In a real scenario, you would include a header file (e.g., zctl.h).
 */
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

  /* * Step 1: Initialize the library
   * Open the target input device.
   */
  printf("Opening device: %s\n", device_path);
  if (zctl_open(device_path) != 0) {
    fprintf(stderr, "Error: Failed to open device '%s'.\n", device_path);
    fprintf(stderr,
            "Hint: Check permissions (try sudo) or if the file exists.\n");
    return 1;
  }

  printf("Successfully opened. Press Ctrl+C to exit.\n");
  printf("Waiting for events...\n");

  uint16_t type, code;
  int32_t value;

  /* * Step 2: Event Loop
   * Poll the device for new events continuously.
   */
  while (1) {
    /* zctl_poll returns 0 if an event was successfully read */
    if (zctl_poll(&type, &code, &value) == 0) {

      /* * Filter and process events.
       * Type 1 corresponds to EV_KEY (Button presses) in Linux input subsystem.
       */
      if (type == 1) {
        printf("[EVENT] Type: EV_KEY (%d) | Code: %d | Value: %d (%s)\n", type,
               code, value, (value == 1) ? "Press" : "Release");
      } else {
        /* Optional: Print other event types (EV_ABS, EV_REL, etc.) */
        /* printf("[EVENT] Type: %d | Code: %d | Value: %d\n", type, code,
         * value); */
      }
    }

    /* * Sleep for a short duration to prevent high CPU usage.
     * 10ms is usually responsive enough for a sample.
     */
    usleep(10000);
  }

  /* * Step 3: Cleanup
   * Close the device when finished (though this loop is infinite,
   * this shows the correct cleanup procedure).
   */
  zctl_close();

  return 0;
}
