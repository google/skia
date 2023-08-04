diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(1) @binding(3) var aSamplerˢ: sampler;
@group(1) @binding(2) var aSamplerᵗ: texture_2d<f32>;
@group(1) @binding(5) var anotherSamplerˢ: sampler;
@group(1) @binding(4) var anotherSamplerᵗ: texture_2d<f32>;
fn helpers_helper_h4Z(sᵗ: texture_2d<f32>, sˢ: sampler) -> vec4<f32> {
  {
    return textureSample(sᵗ, sˢ, vec2<f32>(1.0));
  }
}
fn helper_h4Z(sᵗ: texture_2d<f32>, sˢ: sampler) -> vec4<f32> {
  {
    let _skTemp0 = helpers_helper_h4Z(sᵗ, sˢ);
    return _skTemp0;
  }
}
fn helper2_h4ZZ(s1ᵗ: texture_2d<f32>, s1ˢ: sampler, s2ᵗ: texture_2d<f32>, s2ˢ: sampler) -> vec4<f32> {
  {
    let _skTemp1 = helper_h4Z(s2ᵗ, s2ˢ);
    return textureSample(s1ᵗ, s1ˢ, vec2<f32>(1.0)) + _skTemp1;
  }
}
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = helper_h4Z(aSamplerᵗ, aSamplerˢ);
    let _skTemp3 = helper2_h4ZZ(aSamplerᵗ, aSamplerˢ, anotherSamplerᵗ, anotherSamplerˢ);
    (*_stageOut).sk_FragColor = (textureSample(aSamplerᵗ, aSamplerˢ, vec2<f32>(0.0)) + _skTemp2) + _skTemp3;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
