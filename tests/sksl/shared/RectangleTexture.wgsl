### Compilation failed:

error: :7:22 error: unresolved type 'sampler2D'
var<private> test2D: sampler2D;
                     ^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
var<private> test2D: sampler2D;
var<private> test2DRect: sampler2D;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = sample(test2D, vec2<f32>(0.5));
    (*_stageOut).sk_FragColor = _skTemp0;
    let _skTemp1 = sample(test2DRect, vec2<f32>(0.5));
    (*_stageOut).sk_FragColor = _skTemp1;
    let _skTemp2 = sample(test2DRect, vec3<f32>(0.5));
    (*_stageOut).sk_FragColor = _skTemp2;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
