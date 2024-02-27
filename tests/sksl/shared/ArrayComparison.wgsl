### Compilation failed:

error: :9:14 error: 'uniform' storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  testArray: array<f32, 5>,
             ^^^^^^^^^^^^^

:6:1 note: see layout of struct:
/*            align(16) size(80) */ struct _GlobalUniforms {
/* offset( 0) align(16) size(16) */   colorGreen : vec4<f32>;
/* offset(16) align(16) size(16) */   colorRed : vec4<f32>;
/* offset(32) align( 4) size(20) */   testArray : array<f32, 5>;
/* offset(52) align( 4) size(20) */   testArrayNegative : array<f32, 5>;
/* offset(72) align( 1) size( 8) */   // -- implicit struct size padding --;
/*                               */ };
struct _GlobalUniforms {
^^^^^^

:12:36 note: '_GlobalUniforms' used in address space 'uniform' here
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
                                   ^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testArray: array<f32, 5>,
  testArrayNegative: array<f32, 5>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  x: i32,
  y: i32,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var f1: array<f32, 5> = array<f32, 5>(1.0, 2.0, 3.0, 4.0, 5.0);
    var f2: array<f32, 5> = array<f32, 5>(1.0, 2.0, 3.0, 4.0, 5.0);
    var f3: array<f32, 5> = array<f32, 5>(1.0, 2.0, 3.0, -4.0, 5.0);
    var v1: array<vec3<i32>, 2> = array<vec3<i32>, 2>(vec3<i32>(1, 2, 3), vec3<i32>(4, 5, 6));
    var v2: array<vec3<i32>, 2> = array<vec3<i32>, 2>(vec3<i32>(1, 2, 3), vec3<i32>(4, 5, 6));
    var v3: array<vec3<i32>, 2> = array<vec3<i32>, 2>(vec3<i32>(1, 2, 3), vec3<i32>(4, 5, -6));
    var m1: array<mat2x2<f32>, 3> = array<mat2x2<f32>, 3>(mat2x2<f32>(1.0, 0.0, 0.0, 1.0), mat2x2<f32>(2.0, 0.0, 0.0, 2.0), mat2x2<f32>(3.0, 4.0, 5.0, 6.0));
    var m2: array<mat2x2<f32>, 3> = array<mat2x2<f32>, 3>(mat2x2<f32>(1.0, 0.0, 0.0, 1.0), mat2x2<f32>(2.0, 0.0, 0.0, 2.0), mat2x2<f32>(3.0, 4.0, 5.0, 6.0));
    var m3: array<mat2x2<f32>, 3> = array<mat2x2<f32>, 3>(mat2x2<f32>(1.0, 0.0, 0.0, 1.0), mat2x2<f32>(2.0, 3.0, 4.0, 5.0), mat2x2<f32>(6.0, 0.0, 0.0, 6.0));
    var s1: array<S, 3> = array<S, 3>(S(1, 2), S(3, 4), S(5, 6));
    var s2: array<S, 3> = array<S, 3>(S(1, 2), S(0, 0), S(5, 6));
    var s3: array<S, 3> = array<S, 3>(S(1, 2), S(3, 4), S(5, 6));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((f1[0] == f2[0]) && (f1[1] == f2[1]) && (f1[2] == f2[2]) && (f1[3] == f2[3]) && (f1[4] == f2[4])) && ((f1[0] != f3[0]) || (f1[1] != f3[1]) || (f1[2] != f3[2]) || (f1[3] != f3[3]) || (f1[4] != f3[4]))) && ((_globalUniforms.testArray[0] != _globalUniforms.testArrayNegative[0]) || (_globalUniforms.testArray[1] != _globalUniforms.testArrayNegative[1]) || (_globalUniforms.testArray[2] != _globalUniforms.testArrayNegative[2]) || (_globalUniforms.testArray[3] != _globalUniforms.testArrayNegative[3]) || (_globalUniforms.testArray[4] != _globalUniforms.testArrayNegative[4]))) && ((_globalUniforms.testArray[0] == f1[0]) && (_globalUniforms.testArray[1] == f1[1]) && (_globalUniforms.testArray[2] == f1[2]) && (_globalUniforms.testArray[3] == f1[3]) && (_globalUniforms.testArray[4] == f1[4]))) && ((_globalUniforms.testArray[0] != f3[0]) || (_globalUniforms.testArray[1] != f3[1]) || (_globalUniforms.testArray[2] != f3[2]) || (_globalUniforms.testArray[3] != f3[3]) || (_globalUniforms.testArray[4] != f3[4]))) && ((f1[0] == _globalUniforms.testArray[0]) && (f1[1] == _globalUniforms.testArray[1]) && (f1[2] == _globalUniforms.testArray[2]) && (f1[3] == _globalUniforms.testArray[3]) && (f1[4] == _globalUniforms.testArray[4]))) && ((f3[0] != _globalUniforms.testArray[0]) || (f3[1] != _globalUniforms.testArray[1]) || (f3[2] != _globalUniforms.testArray[2]) || (f3[3] != _globalUniforms.testArray[3]) || (f3[4] != _globalUniforms.testArray[4]))) && (all(v1[0] == v2[0]) && all(v1[1] == v2[1]))) && (any(v1[0] != v3[0]) || any(v1[1] != v3[1]))) && ((all(m1[0][0] == m2[0][0]) && all(m1[0][1] == m2[0][1])) && (all(m1[1][0] == m2[1][0]) && all(m1[1][1] == m2[1][1])) && (all(m1[2][0] == m2[2][0]) && all(m1[2][1] == m2[2][1])))) && ((any(m1[0][0] != m3[0][0]) || any(m1[0][1] != m3[0][1])) || (any(m1[1][0] != m3[1][0]) || any(m1[1][1] != m3[1][1])) || (any(m1[2][0] != m3[2][0]) || any(m1[2][1] != m3[2][1])))) && (((s1[0].x != s2[0].x) || (s1[0].y != s2[0].y)) || ((s1[1].x != s2[1].x) || (s1[1].y != s2[1].y)) || ((s1[2].x != s2[2].x) || (s1[2].y != s2[2].y)))) && (((s3[0].x == s1[0].x) && (s3[0].y == s1[0].y)) && ((s3[1].x == s1[1].x) && (s3[1].y == s1[1].y)) && ((s3[2].x == s1[2].x) && (s3[2].y == s1[2].y)))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
