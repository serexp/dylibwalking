#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t count = _dyld_image_count();
    printf("[+] Total loaded dylibs: %u\n", count);
    for (uint32_t i = 0; i < count; i++) {
        const char *name = _dyld_get_image_name(i);
        const struct mach_header *header = _dyld_get_image_header(i);
        printf("[%2d] 0x%016llX | %s\n", i, (uint64_t)(uintptr_t)header, name);
    }
    return 0;
}
