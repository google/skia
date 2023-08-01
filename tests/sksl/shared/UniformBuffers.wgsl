### Compilation failed:

error: :20:16 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  @size(32) y: array<f32, 2>,
               ^^^^^^^^^^^^^

:17:1 note: see layout of struct:
/*            align(16) size(96) */ struct testBlock {
/* offset( 0) align( 4) size( 4) */   x : f32;
/* offset( 4) align( 4) size(12) */   w : i32;
/* offset(16) align( 4) size(32) */   y : array<f32, 2>;
/* offset(48) align(16) size(48) */   z : _skMatrix33;
/*                               */ };
struct testBlock {
^^^^^^

:23:36 note: 'testBlock' used in address space 'uniform' here
@group(0) @binding(0) var<uniform> _uniform0 : testBlock;
                                   ^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _skRow3 {
    @size(16) r : vec3<f32>
};
struct _skMatrix33 {
    c : array<_skRow3, 3>
};
fn _skMatrixUnpack33(m : _skMatrix33) -> mat3x3<f32> {
    return mat3x3<f32>(m.c[0].r, m.c[1].r, m.c[2].r);
}
struct testBlock {
  @size(4) x: f32,
  @size(12) w: i32,
  @size(32) y: array<f32, 2>,
  z: _skMatrix33,
};
@group(0) @binding(0) var<uniform> _uniform0 : testBlock;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_uniform0.x, _uniform0.y[0], _uniform0.y[1], 0.0);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
