diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var f: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
    var h: array<f32, 4> = f;
    f = h;
    h = f;
    var i3: array<vec3<i32>, 3> = array<vec3<i32>, 3>(vec3<i32>(1), vec3<i32>(2), vec3<i32>(3));
    var s3: array<vec3<i32>, 3> = i3;
    i3 = s3;
    s3 = i3;
    var h2x2: array<mat2x2<f32>, 2> = array<mat2x2<f32>, 2>(mat2x2<f32>(1.0, 2.0, 3.0, 4.0), mat2x2<f32>(5.0, 6.0, 7.0, 8.0));
    var f2x2: array<mat2x2<f32>, 2> = h2x2;
    f2x2 = h2x2;
    h2x2 = f2x2;
    let _skTemp0 = h;
    let _skTemp1 = s3;
    let _skTemp2 = h2x2;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((f[0] == _skTemp0[0]) && (f[1] == _skTemp0[1]) && (f[2] == _skTemp0[2]) && (f[3] == _skTemp0[3])) && (all(i3[0] == _skTemp1[0]) && all(i3[1] == _skTemp1[1]) && all(i3[2] == _skTemp1[2]))) && ((all(f2x2[0][0] == _skTemp2[0][0]) && all(f2x2[0][1] == _skTemp2[0][1])) && (all(f2x2[1][0] == _skTemp2[1][0]) && all(f2x2[1][1] == _skTemp2[1][1])))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
