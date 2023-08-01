diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _skRow2 {
    @size(16) r : vec2<f32>
};
struct _skMatrix22 {
    c : array<_skRow2, 2>
};
fn _skMatrixUnpack22(m : _skMatrix22) -> mat2x2<f32> {
    return mat2x2<f32>(m.c[0].r, m.c[1].r);
}
struct _skMatrix32 {
    c : array<_skRow2, 3>
};
fn _skMatrixUnpack32(m : _skMatrix32) -> mat3x2<f32> {
    return mat3x2<f32>(m.c[0].r, m.c[1].r, m.c[2].r);
}
struct _skMatrix42 {
    c : array<_skRow2, 4>
};
fn _skMatrixUnpack42(m : _skMatrix42) -> mat4x2<f32> {
    return mat4x2<f32>(m.c[0].r, m.c[1].r, m.c[2].r, m.c[3].r);
}
struct UniformBuffer {
  u22: _skMatrix22,
  u23: mat2x3<f32>,
  u24: mat2x4<f32>,
  u32: _skMatrix32,
  u33: mat3x3<f32>,
  u34: mat3x4<f32>,
  u42: _skMatrix42,
  u43: mat4x3<f32>,
  u44: mat4x4<f32>,
};
@group(0) @binding(1) var<uniform> _uniform0 : UniformBuffer;
struct StorageBuffer {
  s22: _skMatrix22,
  s23: mat2x3<f32>,
  s24: mat2x4<f32>,
  s32: _skMatrix32,
  s33: mat3x3<f32>,
  s34: mat3x4<f32>,
  s42: _skMatrix42,
  s43: mat4x3<f32>,
  s44: mat4x4<f32>,
};
@group(0) @binding(2) var<storage, read_write> _storage1 : StorageBuffer;
fn main() -> vec4<f32> {
  {
    return vec4<f32>(0.0);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main();
  return _stageOut;
}
