diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var b: bool = bool(_globalUniforms.unknownInput);
    var b4: vec4<bool> = vec4<bool>(b);
    b4 = vec4<bool>(vec2<bool>(b), false, true);
    b4 = vec4<bool>(false, b, true, false);
    b4 = vec4<bool>(false, b, false, b);
    return vec4<f32>(b4);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
