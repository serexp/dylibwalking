# DO NOT RELASE FOR NOW!

# dylibwalk - PoC for dynamic resolving of functions in the Darwin kernel

## Introduction

`dylibwalk` is a research PoC demonstrating dynamical function resolving via dylib on Darwin (macOS, iOS, etc.). It draws inspiration from Windows PEB walking. \
The goal is to explore runtime dylib discovery and function resolution via these loaded dylibs, primarily for security research, showcasing how import tables are not a reliable way to identify malicious files.

## Concepts

*   **Dylib Enumeration:** `loadeddylibs.c` lists loaded dylibs within a process. This list, managed by `dyld` (the dynamic linker), is essentially Darwin's equivalent to a process's module list on Windows and resides in user space memory.\
      This list, though residing in user-space memory, is RO
*   **Dynamic Function Resolution:** `poc_nslog.c` showcases resolving `NSLog` from the Foundation framework at runtime.  This is similar to `GetProcAddress` on Windows and is vital to evade static analysis.
*    `poc_nslog_minimal.c` showcases the same resolving `NSLog` from the Foundation framework at runtime, but with only the Objective-C runtime.
*   **Payload execution example:** `poc_curl_minimal.c` showcases a HTTPS request using only the Objective-C runtime. It does NOT depend on Foundation framework for compilation.


## PoCs

*   **`darwin/loadeddylibs.c`**: Lists loaded dylibs and their base addresses. Depends on the CRT and obviously dyld.
*   **`darwin/poc_nslog.c`**: Dynamically resolves and calls `NSLog` from Foundation. Depends on the CRT, dyld and Foundation framework at compile-time.

## Technical Details

*   **Architecture:**  Runs on arm64 and x86_64
*   **Minimum OS Versions:**  Tested on macOS 13.0 and iOS 15.0, likely compatible with earlier versions but not explicitly tested.
*   **Dylib List Location:** The list of loaded dylibs is maintained by `dyld` in user space memory. `_dyld_image_count()` and `_dyld_get_image_name`/`_dyld_get_image_header` access this internal data.

## IOCs
The imports of a minimal PoC resolving nslog via this technique are, according to `nm`:
```
[$] aarch64-apple-darwin24.4-nm poc_nslog_manual_resolve
0000000100000000 T __mh_execute_header
                 U _dlclose # used in the PoC for error handling. not strictly necessary.
                 U _dlerror # ^
                 U _dlopen
                 U _dlsym
                 U _objc_getClass # used for NsLog. won't be present if you target another function
                 U _objc_msgSend # ^
                 U _printf # for debugging. 
                 U _sel_registerName # same as objc_getClass
                 U dyld_stub_binder
```
The only significant IOCs of this technique being used in the wild are `dlopen`, `dlsym`, `dyld_stub_binder`.

## Building

A Makefile is provided. Though if you prefer commands:\
`clang loadeddylibs.c -O2`\
`o64-clang poc_nslog.c -framework Foundation -O2`\
It supports building with `osxcross` and clang on MacOS (or *OS).\
Yes, it successfully built on iOS and iPadOS, targetting iOS and iPadOS.
