diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
@group(1) @binding(3) var aSampler_Sampler: sampler;
@group(1) @binding(2) var aSampler_Texture: texture_2d<f32>;
@group(1) @binding(5) var anotherSampler_Sampler: sampler;
@group(1) @binding(4) var anotherSampler_Texture: texture_2d<f32>;
fn helpers_helper_h4Z(s_Texture: texture_2d<f32>, s_Sampler: sampler) -> vec4<f16> {
  {
    return vec4<f16>(textureSample(s_Texture, s_Sampler, vec2<f32>(1.0)));
  }
}
fn helper_h4Z(s_Texture: texture_2d<f32>, s_Sampler: sampler) -> vec4<f16> {
  {
    return helpers_helper_h4Z(s_Texture, s_Sampler);
  }
}
fn helper2_h4ZZ(s1_Texture: texture_2d<f32>, s1_Sampler: sampler, s2_Texture: texture_2d<f32>, s2_Sampler: sampler) -> vec4<f16> {
  {
    return vec4<f16>(textureSample(s1_Texture, s1_Sampler, vec2<f32>(1.0))) + helper_h4Z(s2_Texture, s2_Sampler);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = (vec4<f16>(textureSample(aSampler_Texture, aSampler_Sampler, vec2<f32>(0.0))) + helper_h4Z(aSampler_Texture, aSampler_Sampler)) + helper2_h4ZZ(aSampler_Texture, aSampler_Sampler, anotherSampler_Texture, anotherSampler_Sampler);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
