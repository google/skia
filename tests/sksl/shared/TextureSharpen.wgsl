diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var s_Sampler: sampler;
@group(0) @binding(10001) var s_Texture: texture_2d<f32>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var a: vec4<f32> = vec4<f32>(textureSampleBias(s_Texture, s_Sampler, vec2<f32>(0.0), -0.475));
    let _skTemp2 = vec3<f32>(0.0);
    var b: vec4<f32> = vec4<f32>(textureSampleBias(s_Texture, s_Sampler, _skTemp2.xy / _skTemp2.z, -0.475));
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(a.xy), vec2<f32>(b.xy));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
