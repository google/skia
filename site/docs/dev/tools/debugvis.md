
---
title: "Debug Visualization"
linkTitle: "Debug Visualization"

---


Skia uses custom container types, such as `SkString` and `SkTArray<>`, which can
be inconvenient to view in a debugger.

If you frequently debug code that uses Skia types, consider installing a debug
visualizer. Skia offers debugger visualization support for the following
platforms:

-   [Visual Studio and VS Code](https://skia.googlesource.com/skia/+/refs/heads/main/platform_tools/debugging/vs/Skia.natvis)
-   [LLDB and Xcode](https://skia.googlesource.com/skia/+/refs/heads/main/platform_tools/debugging/lldb/skia.py)

