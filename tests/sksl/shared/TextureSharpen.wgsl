### Compilation failed:

error: :7:17 error: unresolved type 'sampler2D'
var<private> s: sampler2D;
                ^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
var<private> s: sampler2D;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = sample(s, vec2<f32>(0.0));
    var a: vec4<f32> = vec4<f32>(_skTemp0);
    let _skTemp1 = sample(s, vec3<f32>(0.0));
    var b: vec4<f32> = vec4<f32>(_skTemp1);
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(a.xy), vec2<f32>(b.xy));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
