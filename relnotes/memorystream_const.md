`SkMemoryStream` now takes in a `const SkData`, as it's a read-only view into that data.
`SkMemory::getData()` now returns a `const sk_sp<SkData>`.