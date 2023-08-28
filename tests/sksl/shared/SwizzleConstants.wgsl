diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var v: vec4<f32> = _globalUniforms.testInputs;
    v = vec4<f32>(v.x, 1.0, 1.0, 1.0);
    v = vec4<f32>(v.xy, 1.0, 1.0);
    v = vec4<f32>(v.x, 1.0, 1.0, 1.0);
    v = vec4<f32>(0.0, v.y, 1.0, 1.0);
    v = vec4<f32>(v.xyz, 1.0);
    v = vec4<f32>(v.xy, 1.0, 1.0);
    v = vec4<f32>(v.x, 0.0, v.z, 1.0);
    v = vec4<f32>(v.x, 1.0, 0.0, 1.0);
    v = vec4<f32>(1.0, v.yz, 1.0);
    v = vec4<f32>(0.0, v.y, 1.0, 1.0);
    v = vec4<f32>(1.0, 1.0, v.z, 1.0);
    v = vec4<f32>(v.xyz, 1.0);
    v = vec4<f32>(v.xy, 0.0, v.w);
    v = vec4<f32>(v.xy, 1.0, 0.0);
    v = vec4<f32>(v.x, 1.0, v.zw);
    v = vec4<f32>(v.x, 0.0, v.z, 1.0);
    v = vec4<f32>(v.x, 1.0, 1.0, v.w);
    v = vec4<f32>(v.x, 1.0, 0.0, 1.0);
    v = vec4<f32>(1.0, v.yzw);
    v = vec4<f32>(0.0, v.yz, 1.0);
    v = vec4<f32>(0.0, v.y, 1.0, v.w);
    v = vec4<f32>(1.0, v.y, 1.0, 1.0);
    v = vec4<f32>(0.0, 0.0, v.zw);
    v = vec4<f32>(0.0, 0.0, v.z, 1.0);
    v = vec4<f32>(0.0, 1.0, 1.0, v.w);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(v == vec4<f32>(0.0, 1.0, 1.0, 1.0))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
