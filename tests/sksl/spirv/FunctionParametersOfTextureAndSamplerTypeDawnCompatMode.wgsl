diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @location(1) c: vec2<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(1) var aTexture: texture_2d<f32>;
@group(0) @binding(3) var aSampledTextureˢ: sampler;
@group(0) @binding(2) var aSampledTextureᵗ: texture_2d<f32>;
fn helpers_helper_h4ZT(_stageIn: FSIn, sᵗ: texture_2d<f32>, sˢ: sampler, t: texture_2d<f32>) -> vec4<f32> {
  {
    return textureSample(sᵗ, sˢ, _stageIn.c);
  }
}
fn helper_h4TZ(_stageIn: FSIn, t: texture_2d<f32>, sᵗ: texture_2d<f32>, sˢ: sampler) -> vec4<f32> {
  {
    let _skTemp0 = helpers_helper_h4ZT(_stageIn, sᵗ, sˢ, t);
    return _skTemp0;
  }
}
fn main(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    let _skTemp1 = helper_h4TZ(_stageIn, aTexture, aSampledTextureᵗ, aSampledTextureˢ);
    (*_stageOut).sk_FragColor = _skTemp1;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(_stageIn, &_stageOut);
  return _stageOut;
}
