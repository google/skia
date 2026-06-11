---
title: "Dangling Pointer Detector"
linkTitle: "Dangling Pointer Detector"

---

A pointer is dangling when it references freed memory. Dangling pointers are a source of Use-After-Free (UAF) bugs and are highly discouraged unless you can definitively ensure that they will never be dereferenced or used after the pointed-to object is freed.

Skia tests (run via `dm`) are configured to detect dangling `raw_ptr<T>` instances when built with PartitionAlloc enabled (`skia_use_partition_alloc = true`).

## Motivation

Tracking the lifetime of interacting objects across a complex C++ codebase is difficult. Often, lifetime issues are discovered late when they cause hard-to-reproduce user crashes.

Enforcing the Dangling Pointer Detector on the Commit Queue (CQ) helps us:
1. **Prevent Regressions**: Catch accidental lifetime changes that invalidate prior ownership assumptions before they ship.
2. **Promote Better Architecture**: Flag ambiguous object lifetimes during code review.
3. **Verify Cleanups**: Give developers immediate confirmation when cleaning up dangling pointers.

## `raw_ptr<T>`

A `raw_ptr<T>` is a non-owning smart pointer. When using `raw_ptr<T>`, the severity of UAFs is significantly mitigated because the underlying allocation is protected by [MiraclePtr / BackupRefPtr](https://security.googleblog.com/2022/09/use-after-freedom-miracleptr.html).

A `raw_ptr<T>` works transparently like a raw `T*`. It should primarily be used for class and struct member variables.

## Flavors & Annotations

When a pointer must temporarily dangle safely, or represents an untriaged legacy instance, you can use these annotations:

```cpp
raw_ptr<T> ptr_never_dangling;
raw_ptr<T, DisableDanglingPtrDetection> ptr_allowed_to_dangle;
raw_ptr<T, DanglingUntriaged> ptr_dangling_to_investigate;
```

* `DisableDanglingPtrDetection`: Used to annotate intentional and safe dangling pointers as a last resort if re-architecting ownership is impractical.
* `DanglingUntriaged`: Indicates a pre-existing dangling pointer marked for future cleanup.
