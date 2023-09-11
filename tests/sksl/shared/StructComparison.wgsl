### Compilation failed:

error: :8:14 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  testArray: array<f32, 5>,
             ^^^^^^^^^^^^^

:5:1 note: see layout of struct:
/*            align(16) size(64) */ struct _GlobalUniforms {
/* offset( 0) align(16) size(16) */   colorGreen : vec4<f32>;
/* offset(16) align(16) size(16) */   colorRed : vec4<f32>;
/* offset(32) align( 4) size(20) */   testArray : array<f32, 5>;
/* offset(52) align( 1) size(12) */   // -- implicit struct size padding --;
/*                               */ };
struct _GlobalUniforms {
^^^^^^

:10:36 note: '_GlobalUniforms' used in address space 'uniform' here
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
                                   ^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testArray: array<f32, 5>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  x: i32,
  y: i32,
  m: mat2x2<f32>,
  a: array<f32, 5>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var R_array: array<f32, 5> = array<f32, 5>(1.0, 2.0, 3.0, 4.0, 5.0);
    var s1: S = S(1, 2, mat2x2<f32>(1.0, 0.0, 0.0, 1.0), R_array);
    var s2: S = S(1, 2, mat2x2<f32>(1.0, 0.0, 0.0, 1.0), _globalUniforms.testArray);
    var s3: S = S(1, 2, mat2x2<f32>(2.0, 0.0, 0.0, 2.0), array<f32, 5>(1.0, 2.0, 3.0, 4.0, 5.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((s1.x == s2.x) && (s1.y == s2.y) && (all(s1.m[0] == s2.m[0]) && all(s1.m[1] == s2.m[1])) && ((s1.a[0] == s2.a[0]) && (s1.a[1] == s2.a[1]) && (s1.a[2] == s2.a[2]) && (s1.a[3] == s2.a[3]) && (s1.a[4] == s2.a[4]))) && ((s1.x != s3.x) || (s1.y != s3.y) || (any(s1.m[0] != s3.m[0]) || any(s1.m[1] != s3.m[1])) || ((s1.a[0] != s3.a[0]) || (s1.a[1] != s3.a[1]) || (s1.a[2] != s3.a[2]) || (s1.a[3] != s3.a[3]) || (s1.a[4] != s3.a[4])))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
