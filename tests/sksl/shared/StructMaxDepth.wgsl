diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct S1 {
  x: i32,
};
struct S2 {
  x: S1,
};
struct S3 {
  x: S2,
};
struct S4 {
  x: S3,
};
struct S5 {
  x: S4,
};
struct S6 {
  x: S5,
};
struct S7 {
  x: S6,
};
struct S8 {
  x: S7,
};
struct SA1 {
  x: array<i32, 2>,
};
struct SA2 {
  x: array<SA1, 2>,
};
struct SA3 {
  x: array<SA2, 2>,
};
struct SA4 {
  x: array<SA3, 2>,
};
struct SA5 {
  x: array<SA4, 2>,
};
struct SA6 {
  x: array<SA5, 2>,
};
struct SA7 {
  x: array<SA6, 2>,
};
struct SA8 {
  x: array<SA7, 2>,
};
