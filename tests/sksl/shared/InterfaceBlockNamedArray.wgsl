### Compilation failed:

error: :9:47 error: unresolved identifier '_globalUniforms'
    (*_stageOut).sk_FragColor = vec4<f32>(f32(_globalUniforms.test[1].x));
                                              ^^^^^^^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(f32(_globalUniforms.test[1].x));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
