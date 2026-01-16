diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  colorBlack: vec4<f32>,
  colorWhite: vec4<f32>,
  testInputs: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let FTFT: vec4<bool> = vec4<bool>(_globalUniforms.colorGreen);
    let TFTF: vec4<bool> = FTFT.wzyx;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((select(_globalUniforms.colorBlack.x, _globalUniforms.colorWhite.x, FTFT.x) == _globalUniforms.colorBlack.x) && all(select(_globalUniforms.colorBlack.xy, _globalUniforms.colorWhite.xy, FTFT.xy) == vec2<f32>(_globalUniforms.colorBlack.x, 1.0))) && all(select(_globalUniforms.colorBlack.xyz, _globalUniforms.colorWhite.xyz, FTFT.xyz) == vec3<f32>(_globalUniforms.colorBlack.x, 1.0, _globalUniforms.colorBlack.z))) && all(select(_globalUniforms.colorBlack, _globalUniforms.colorWhite, FTFT) == vec4<f32>(_globalUniforms.colorBlack.x, 1.0, _globalUniforms.colorBlack.z, 1.0))) && (select(_globalUniforms.colorWhite.x, _globalUniforms.testInputs.x, TFTF.x) == _globalUniforms.testInputs.x)) && all(select(_globalUniforms.colorWhite.xy, _globalUniforms.testInputs.xy, TFTF.xy) == vec2<f32>(_globalUniforms.testInputs.x, 1.0))) && all(select(_globalUniforms.colorWhite.xyz, _globalUniforms.testInputs.xyz, TFTF.xyz) == vec3<f32>(_globalUniforms.testInputs.x, 1.0, _globalUniforms.testInputs.z))) && all(select(_globalUniforms.colorWhite, _globalUniforms.testInputs, TFTF) == vec4<f32>(_globalUniforms.testInputs.x, 1.0, _globalUniforms.testInputs.z, 1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
