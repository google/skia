diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var h4: vec4<f32> = vec4<f32>(_globalUniforms.unknownInput);
    h4 = vec4<f32>(vec2<f32>(_globalUniforms.unknownInput), 0.0, 1.0);
    h4 = vec4<f32>(0.0, _globalUniforms.unknownInput, 1.0, 0.0);
    h4 = vec4<f32>(0.0, _globalUniforms.unknownInput, 0.0, _globalUniforms.unknownInput);
    return h4;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
