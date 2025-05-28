# dylibwalk - PoC for dynamic resolving of functions in the Darwin kernel

## Introduction

`dylibwalk` is a research PoC demonstrating dynamical function resolving via runtime dylib inspection on Darwin (macOS, iOS, etc.). It draws inspiration from Windows PEB walking. \
The goal is to explore runtime dylib discovery and function resolution via these loaded dylibs, primarily for security research, showcasing how import tables are not a reliable way to identify malicious files.

## Concepts

*   **Dylib Enumeration:** `loadeddylibs.c` lists loaded dylibs within a process. This list, managed by `dyld` (the dynamic linker), is essentially Darwin's way to keep track of a process's loaded module list. It resides in user space memory and is read-only.\
*   **Dynamic Function Resolution:** `poc_nslog.c` showcases resolving `NSLog` from the Foundation framework at runtime, therefore only depending on only the Objective-C runtime.
*   **Payload execution example:** We wish to clarify that there is nothing preventing motivated actors from using this to execute arbitrary code (think malware).

## PoCs

*   **`loadeddylibs.c`**: Lists loaded dylibs and their base addresses. Depends on the CRT and naturally, dyld.
*   **`poc_nslog.c`**: Dynamically resolves and calls `NSLog` from Foundation. Depends on the CRT, dyld and Foundation framework at compile-time.
*   **`walker.c`: Experimental NSLog PoC that doesn't use dlsym/dlopen. Requires fixing, released as study material.
*   *Further PoCs will be released. A cURL PoC is in the works. :)*

## Technical Details

*   **Architecture:**  Runs on arm64 and x86_64/universal.
*   **Minimum OS Versions:**  Tested on macOS 13.0 and iOS 15.0, likely compatible with earlier versions but not explicitly tested.
*   **Dylib List Location:** The list of loaded dylibs is maintained by `dyld` in user space memory. `_dyld_image_count()` and `_dyld_get_image_name`/`_dyld_get_image_header` grant access to this internal data.


## Indicators Of Compromise (IOCs)
The imports of a minimal PoC resolving nslog via this technique are, according to `nm`:
```
[$] aarch64-apple-darwin24.4-nm ./poc_nslog
0000000100000000 T __mh_execute_header
                 U _dlclose # used in the PoC for error handling. not strictly necessary.
                 U _dlerror # ^
                 U _dlopen
                 U _dlsym
                 U _objc_getClass # used for NSLog. won't be present if you target another function
                 U _objc_msgSend # ^
                 U _printf # for debugging.
                 U _sel_registerName # same as objc_getClass
                 U dyld_stub_binder
```
The only significant IOCs of this technique being used in the wild are `dlopen`, `dlsym`, `dyld_stub_binder`, which can all be bypassed and enumerated at runtime by a motivated actor.

## Building

A Makefile is provided. Though if you prefer commands:\
`clang loadeddylibs.c -O2`\
`clang poc_nslog.c -framework Foundation -O2`\
`clang walker.c -nostdlib -lobjc -lc -Wall -O3 -std=c99`
Ssssssssssssssssssssssssssssssssssssssupports building with `osxcross` and clang on MacOS (or *OS).\
Successfully built on iOS and iPadOS, targetting iOS and iPadOS.
