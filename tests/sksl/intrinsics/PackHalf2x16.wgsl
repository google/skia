diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
  testInputs: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let xy: u32 = pack2x16float(_globalUniforms.testInputs.xy);
    let zw: u32 = pack2x16float(_globalUniforms.testInputs.zw);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(unpack2x16float(xy) == vec2<f32>(-1.25, 0.0)) && all(unpack2x16float(zw) == vec2<f32>(0.75, 2.25))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
