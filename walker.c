#include <dlfcn.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <string.h>
#include <stdint.h>
#include <mach-o/dyld.h>

extern id objc_msgSend(id self, SEL op, ...);
typedef void (*NSLogFunc)(id format, ...);
const char* foundationFrameworkPath = "/System/Library/Frameworks/Foundation.framework/Foundation";

/* Function to manually resolve a symbol address from a Mach-O module base address. Needs rework. */
void* DarwinGetProcAddress(void* moduleBase, const char* procName) {
    if (!moduleBase || !procName) return NULL;

    struct mach_header_64 *header = (struct mach_header_64 *)moduleBase;
    if (header->magic != MH_MAGIC_64 && header->magic != MH_CIGAM_64) {
        header = (struct mach_header_64 *)((char *)moduleBase + 16384); /* Try slide for ASLR (minimal PoC attempt) */ 
        if (header->magic != MH_MAGIC_64 && header->magic != MH_CIGAM_64)
            return NULL; /* Check magic number (64-bit Mach-O) */ 
    }


    struct load_command *load_cmd = (struct load_command *)((char *)moduleBase + sizeof(struct mach_header_64));
    struct symtab_command *symtab_cmd = NULL;

    /* Find LC_SYMTAB load command */
    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (load_cmd->cmd == LC_SYMTAB) {
            symtab_cmd = (struct symtab_command *)load_cmd;
            break;
        }
        load_cmd = (struct load_command *)((char *)load_cmd + load_cmd->cmdsize);
    }

    if (!symtab_cmd) return NULL; /* No symbol table :( */

    struct nlist_64 *symtab = (struct nlist_64 *)((char *)moduleBase + symtab_cmd->symoff);
    char *strtab = (char *)moduleBase + symtab_cmd->stroff;

    for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
        char *symbol_name = strtab + symtab[i].n_un.n_strx;
        if (strcmp(symbol_name, procName) == 0) {
            /* Basic check: exported and in a section (function) - TODO: improve */
            if ((symtab[i].n_type & N_TYPE) == N_SECT && (symtab[i].n_type & N_EXT)) {
                return (void*)((char *)moduleBase + symtab[i].n_value);
            }
        }
    }

    return NULL; /* Function not found */
}


int main() {
    /* Load Foundation framework using dlopen (still needed to get the base address initially, or use dyld functions). TODO: fix. */
    void *foundation = dlopen(foundationFrameworkPath, RTLD_LAZY);
    printf("[+] Loading Foundation framework from %s...\n", foundationFrameworkPath);
    if (!foundation) {
        printf("[!] dlopen error: %s\nFoundation framework loading failed!.\n", dlerror());
        goto exit;
    }

    void *foundationBaseAddress = NULL;
    uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        const char *name = _dyld_get_image_name(i);
        /* TODO: fix to be a bit more specific to avoid matching other things containing "Foundation" (i.e: CoreFoundation)*/
        if (name && strstr(name, "/Foundation.framework/Foundation") != NULL) {
            foundationBaseAddress = (void*)_dyld_get_image_header(i);
            printf("[+] Found Foundation via dyld functions. Base address: %p\n", foundationBaseAddress);
            break;
        }
    }

    if (!foundationBaseAddress) {
        printf("[!] Failed to find Foundation base address using _dyld functions.\n");
        dlclose(foundation);
        goto exit;
    }

    NSLogFunc NSLog_ptr_manual = (NSLogFunc)DarwinGetProcAddress(foundationBaseAddress, "NSLog");
    if (!NSLog_ptr_manual) {
        printf("[!] DarwinGetProcAddress error: Failed to resolve NSLog manually.\n");
        dlclose(foundation);
        goto exit;
    }

    printf("[+] Manually resolved NSLog address: %p\n", NSLog_ptr_manual);
    Class NSString = objc_getClass("NSString");
    SEL selector = sel_registerName("stringWithUTF8String:");
    id message = objc_msgSend((id)NSString, selector, "[+] Dynamically called NSLog via manual Mach-O parsing!");
    NSLog_ptr_manual(message);


    dlclose(foundation);
    printf("[+] Done.\n");
    return 0;

exit:
    printf("Exiting...\n");
    return 404;
}
