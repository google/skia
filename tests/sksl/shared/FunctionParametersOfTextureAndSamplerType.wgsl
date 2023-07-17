### Compilation failed:

error: :9:24 error: unresolved type 'texture2D'
var<private> aTexture: texture2D;
                       ^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @location(1) c: vec2<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
var<private> aTexture: texture2D;
var<private> aSampledTexture: sampler2D;
fn helpers_helper_h4ZT(_stageIn: FSIn, _skParam0: sampler2D, _skParam1: texture2D) -> vec4<f32> {
  let s = _skParam0;
  let t = _skParam1;
  {
    let _skTemp0 = sample(s, _stageIn.c);
    return _skTemp0;
  }
}
fn helper_h4TZ(_stageIn: FSIn, _skParam0: texture2D, _skParam1: sampler2D) -> vec4<f32> {
  let t = _skParam0;
  let s = _skParam1;
  {
    let _skTemp1 = helpers_helper_h4ZT(_stageIn, s, t);
    return _skTemp1;
  }
}
fn main(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = helper_h4TZ(_stageIn, aTexture, aSampledTexture);
    (*_stageOut).sk_FragColor = _skTemp2;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(_stageIn, &_stageOut);
  return _stageOut;
}

1 error
