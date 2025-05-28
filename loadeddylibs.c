#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    // Get the number of loaded dylibs
    uint32_t count = _dyld_image_count();
    printf("[+] Total loaded dylibs: %u\n", count);

    // Iterate through all loaded dylibs
    for (uint32_t i = 0; i < count; i++) {
        // Get the dylib's name (path)
        const char *name = _dyld_get_image_name(i);
        // Get the dylib's base address (header)
        const struct mach_header *header = _dyld_get_image_header(i);
        // Print formatted output
        printf("[%2d] 0x%016llX | %s\n", i, (uint64_t)(uintptr_t)header, name);
    }

    return 0;
}
