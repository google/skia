### Compilation failed:

error: :15:20 error: unresolved call target 'bitCount'
    let _skTemp0 = bitCount(_globalUniforms.a);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: i32,
  b: u32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = bitCount(_globalUniforms.a);
    (*_stageOut).sk_FragColor.x = f32(_skTemp0);
    let _skTemp1 = bitCount(_globalUniforms.b);
    (*_stageOut).sk_FragColor.y = f32(_skTemp1);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
