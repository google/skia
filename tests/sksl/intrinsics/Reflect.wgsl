diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  I: vec4<f16>,
  N: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    let _skTemp0 = 65504.0h;
    let _skTemp1 = (_skTemp0 * 222.0h) * 2.0h;
    let _skTemp2 = -65504.0h;
    var expectedX: f16 = (_skTemp1 - 2 * _skTemp2 * _skTemp1 * _skTemp2);
    expectedX = -49.0h;
    const expectedXY: vec2<f16> = vec2<f16>(-169.0h, 202.0h);
    const expectedXYZ: vec3<f16> = vec3<f16>(-379.0h, 454.0h, -529.0h);
    const expectedXYZW: vec4<f16> = vec4<f16>(-699.0h, 838.0h, -977.0h, 1116.0h);
    let _skTemp3 = _globalUniforms.I.x;
    let _skTemp4 = _globalUniforms.N.x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((_skTemp3 - 2 * _skTemp4 * _skTemp3 * _skTemp4) == expectedX) && all(reflect(_globalUniforms.I.xy, _globalUniforms.N.xy) == expectedXY)) && all(reflect(_globalUniforms.I.xyz, _globalUniforms.N.xyz) == expectedXYZ)) && all(reflect(_globalUniforms.I, _globalUniforms.N) == expectedXYZW)) && (-49.0h == expectedX)) && all(vec2<f16>(-169.0h, 202.0h) == expectedXY)) && all(vec3<f16>(-379.0h, 454.0h, -529.0h) == expectedXYZ)) && all(vec4<f16>(-699.0h, 838.0h, -977.0h, 1116.0h) == expectedXYZW)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
