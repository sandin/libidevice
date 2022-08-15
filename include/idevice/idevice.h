#ifndef IDEVICE_IDEVICE_H
#define IDEVICE_IDEVICE_H

#include <cstdint>
#include <cstdio>

namespace idevice {

static const int32_t kResultOk = 0;
static const int32_t kResultUnknownError = -256;

static const char* kClientLabel = "IDevice";

static inline void hexdump(void* addr, int len, int offset) {
  int i;
  unsigned char buff[17];
  unsigned char* pc = (unsigned char*)addr;

  // Process every byte in the data.
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).

    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0) printf("  %s\n", buff);

      // Output the offset.
      printf("  %08x ", i + offset);
    }

    // Now the hex code for the specific character.
    printf(" %02x", pc[i]);

    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
      buff[i % 16] = '.';
    } else {
      buff[i % 16] = pc[i];
    }

    buff[(i % 16) + 1] = '\0';
  }

  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf("   ");
    i++;
  }

  // And print the final ASCII bit.
  printf("  %s\n", buff);
}

}  // namespace idevice

#endif // IDEVICE_IDEVICE_H
