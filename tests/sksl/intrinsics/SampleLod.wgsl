diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var t_Sampler: sampler;
@group(0) @binding(10001) var t_Texture: texture_2d<f32>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var c: vec4<f32> = textureSampleLevel(t_Texture, t_Sampler, vec2<f32>(0.0), 0.0);
    let _skTemp2 = vec3<f32>(1.0);
    (*_stageOut).sk_FragColor = c * textureSampleLevel(t_Texture, t_Sampler, _skTemp2.xy / _skTemp2.z, 0.0);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
