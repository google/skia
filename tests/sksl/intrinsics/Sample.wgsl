diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var t_Sampler: sampler;
@group(0) @binding(10001) var t_Texture: texture_2d<f32>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var c: vec4<f32> = textureSample(t_Texture, t_Sampler, vec2<f32>(0.0));
    let _skTemp2 = vec3<f32>(1.0);
    (*_stageOut).sk_FragColor = c * textureSample(t_Texture, t_Sampler, _skTemp2.xy / _skTemp2.z);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
