diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix2x2: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    ok = ok && all(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    ok = ok && all(vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    ok = ok && all(vec4<i32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) == vec4<i32>(1, 2, 3, 4));
    ok = ok && all(vec4<bool>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) == vec4<bool>(true));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
