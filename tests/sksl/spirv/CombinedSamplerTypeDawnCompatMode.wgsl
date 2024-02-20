diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(1) @binding(3) var aSampler_Sampler: sampler;
@group(1) @binding(2) var aSampler_Texture: texture_2d<f32>;
@group(1) @binding(5) var anotherSampler_Sampler: sampler;
@group(1) @binding(4) var anotherSampler_Texture: texture_2d<f32>;
fn helpers_helper_h4Z(s_Texture: texture_2d<f32>, s_Sampler: sampler) -> vec4<f32> {
  {
    return textureSample(s_Texture, s_Sampler, vec2<f32>(1.0));
  }
}
fn helper_h4Z(s_Texture: texture_2d<f32>, s_Sampler: sampler) -> vec4<f32> {
  {
    let _skTemp0 = helpers_helper_h4Z(s_Texture, s_Sampler);
    return _skTemp0;
  }
}
fn helper2_h4ZZ(s1_Texture: texture_2d<f32>, s1_Sampler: sampler, s2_Texture: texture_2d<f32>, s2_Sampler: sampler) -> vec4<f32> {
  {
    let _skTemp1 = helper_h4Z(s2_Texture, s2_Sampler);
    return textureSample(s1_Texture, s1_Sampler, vec2<f32>(1.0)) + _skTemp1;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = helper_h4Z(aSampler_Texture, aSampler_Sampler);
    let _skTemp3 = helper2_h4ZZ(aSampler_Texture, aSampler_Sampler, anotherSampler_Texture, anotherSampler_Sampler);
    (*_stageOut).sk_FragColor = (textureSample(aSampler_Texture, aSampler_Sampler, vec2<f32>(0.0)) + _skTemp2) + _skTemp3;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
