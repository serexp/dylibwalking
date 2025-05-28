#include <dlfcn.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <stdio.h>

extern id objc_msgSend(id self, SEL op, ...);
typedef void (*NSLog_t)(id format, ...);
const char* foundationFrameworkPath = "/System/Library/Frameworks/Foundation.framework/Foundation";

int main() {
    const char* targetFunctionName = "NSLog";
    void *foundation = dlopen(foundationFrameworkPath, RTLD_NOW); /* RTLD_LAZY); */
    printf("[+] Loading Foundation framework from %s...\n", foundationFrameworkPath);
    if (!foundation) {
        printf("[!] dlopen error: %s\nFoundation framework loading failed!\n", dlerror());
        goto exit;
    }

    NSLog_t __NSLog = (NSLog_t)dlsym(foundation, targetFunctionName);
    if (!__NSLog) {
        printf("[!] dlsym error: %s\nFailed to retrieve %s pointer. Try with a different function?\n", dlerror(), targetFunctionName);
        dlclose(foundation);
        goto exit;
    }

    Class NSString = objc_getClass("NSString");
    SEL selector = sel_registerName("stringWithUTF8String:");
    id message = objc_msgSend((id)NSString, selector, "[+] Dynamically resolved NSLog!");
    __NSLog(message);
    dlclose(foundation);
    return 0;

exit:
    printf("[+] Exiting...");
    return 99;
}
/*
When stripped:

[$] aarch64-apple-darwin24.4-nm a.out
0000000100000000 T __mh_execute_header # no NSLog!!!
                 U _dlclose
                 U _dlerror
                 U _dlopen
                 U _dlsym
                 U _objc_getClass
                 U _objc_msgSend
                 U _printf
                 U _sel_registerName
                 U dyld_stub_binder

[$] llvm-otool -L a.out
a.out:
	/System/Library/Frameworks/Foundation.framework/Versions/C/Foundation (compatibility version 300.0.0, current version 3423.0.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1351.0.0)
	/usr/lib/libobjc.A.dylib (compatibility version 1.0.0, current version 228.0.0)

*/
