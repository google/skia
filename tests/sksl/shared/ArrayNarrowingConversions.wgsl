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
    var i2: array<i32, 2> = array<i32, 2>(1, 2);
    var s2: array<i32, 2> = array<i32, 2>(1, 2);
    var f2: array<f32, 2> = array<f32, 2>(1.0, 2.0);
    var h2: array<f16, 2> = array<f16, 2>(1.0h, 2.0h);
    i2 = s2;
    s2 = i2;
    f2 = array<f32, 2>(f32(h2[0]), f32(h2[1]));
    h2 = array<f16, 2>(f16(f2[0]), f16(f2[1]));
    const cf2: array<f32, 2> = array<f32, 2>(1.0, 2.0);
    let _skTemp0 = s2;
    let _skTemp1 = array<f32, 2>(f32(h2[0]), f32(h2[1]));
    const _skTemp2 = array<i32, 2>(1, 2);
    let _skTemp3 = array<f32, 2>(f32(h2[0]), f32(h2[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((i2[0] == _skTemp0[0]) && (i2[1] == _skTemp0[1])) && ((f2[0] == _skTemp1[0]) && (f2[1] == _skTemp1[1]))) && ((i2[0] == _skTemp2[0]) && (i2[1] == _skTemp2[1]))) && ((_skTemp3[0] == cf2[0]) && (_skTemp3[1] == cf2[1]))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
