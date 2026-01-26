diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var f: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
    var h: array<f16, 4> = array<f16, 4>(f16(f[0]), f16(f[1]), f16(f[2]), f16(f[3]));
    f = array<f32, 4>(f32(h[0]), f32(h[1]), f32(h[2]), f32(h[3]));
    h = array<f16, 4>(f16(f[0]), f16(f[1]), f16(f[2]), f16(f[3]));
    var i3: array<vec3<i32>, 3> = array<vec3<i32>, 3>(vec3<i32>(1), vec3<i32>(2), vec3<i32>(3));
    var s3: array<vec3<i32>, 3> = i3;
    i3 = s3;
    s3 = i3;
    var h2x2: array<mat2x2<f16>, 2> = array<mat2x2<f16>, 2>(mat2x2<f16>(1.0h, 2.0h, 3.0h, 4.0h), mat2x2<f16>(5.0h, 6.0h, 7.0h, 8.0h));
    var f2x2: array<mat2x2<f32>, 2> = array<mat2x2<f32>, 2>(mat2x2<f32>(h2x2[0]), mat2x2<f32>(h2x2[1]));
    f2x2 = array<mat2x2<f32>, 2>(mat2x2<f32>(h2x2[0]), mat2x2<f32>(h2x2[1]));
    h2x2 = array<mat2x2<f16>, 2>(mat2x2<f16>(f2x2[0]), mat2x2<f16>(f2x2[1]));
    let _skTemp0 = array<f32, 4>(f32(h[0]), f32(h[1]), f32(h[2]), f32(h[3]));
    let _skTemp1 = s3;
    let _skTemp2 = array<mat2x2<f32>, 2>(mat2x2<f32>(h2x2[0]), mat2x2<f32>(h2x2[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((f[0] == _skTemp0[0]) && (f[1] == _skTemp0[1]) && (f[2] == _skTemp0[2]) && (f[3] == _skTemp0[3])) && (all(i3[0] == _skTemp1[0]) && all(i3[1] == _skTemp1[1]) && all(i3[2] == _skTemp1[2]))) && ((all(f2x2[0][0] == _skTemp2[0][0]) && all(f2x2[0][1] == _skTemp2[0][1])) && (all(f2x2[1][0] == _skTemp2[1][0]) && all(f2x2[1][1] == _skTemp2[1][1])))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
