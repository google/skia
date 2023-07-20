### Compilation failed:

error: :11:6 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  y: array<f32, 2>,
     ^^^^^^^^^^^^^

:8:1 note: see layout of struct:
/*            align(16) size(64) */ struct testBlock {
/* offset( 0) align( 4) size( 4) */   x : f32;
/* offset( 4) align( 4) size( 4) */   w : i32;
/* offset( 8) align( 4) size( 8) */   y : array<f32, 2>;
/* offset(16) align(16) size(48) */   z : mat3x3<f32>;
/*                               */ };
struct testBlock {
^^^^^^

:14:36 note: 'testBlock' used in address space 'uniform' here
@group(0) @binding(0) var<uniform> _uniforms : testBlock;
                                   ^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlock {
  x: f32,
  w: i32,
  y: array<f32, 2>,
  z: mat3x3<f32>,
};
@group(0) @binding(0) var<uniform> _uniforms : testBlock;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_uniforms.x, _uniforms.y[0], _uniforms.y[1], 0.0);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
