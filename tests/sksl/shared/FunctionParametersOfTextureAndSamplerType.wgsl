diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSIn {
  @location(1) c: vec2<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(1) var aTexture: texture_2d<f32>;
@group(0) @binding(10000) var aSampledTexture_Sampler: sampler;
@group(0) @binding(10001) var aSampledTexture_Texture: texture_2d<f32>;
@group(0) @binding(10002) var aSecondSampledTexture_Sampler: sampler;
@group(0) @binding(10003) var aSecondSampledTexture_Texture: texture_2d<f32>;
fn helpers_helper_h4ZT(_stageIn: FSIn, s_Texture: texture_2d<f32>, s_Sampler: sampler, t: texture_2d<f32>) -> vec4<f32> {
  {
    return textureSample(s_Texture, s_Sampler, _stageIn.c);
  }
}
fn helper_h4TZ(_stageIn: FSIn, t: texture_2d<f32>, s_Texture: texture_2d<f32>, s_Sampler: sampler) -> vec4<f32> {
  {
    let _skTemp4 = helpers_helper_h4ZT(_stageIn, s_Texture, s_Sampler, t);
    return _skTemp4;
  }
}
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    let _skTemp5 = helper_h4TZ(_stageIn, aTexture, aSampledTexture_Texture, aSampledTexture_Sampler);
    (*_stageOut).sk_FragColor = _skTemp5;
    let _skTemp6 = helper_h4TZ(_stageIn, aTexture, aSecondSampledTexture_Texture, aSecondSampledTexture_Sampler);
    (*_stageOut).sk_FragColor = _skTemp6;
    let _skTemp7 = helper_h4TZ(_stageIn, aTexture, aSampledTexture_Texture, aSampledTexture_Sampler);
    (*_stageOut).sk_FragColor = _skTemp7;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
