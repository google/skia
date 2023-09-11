### Compilation failed:

error: :12:20 error: unresolved call target 'findMSB'
    let _skTemp0 = findMSB(_globalUniforms.a);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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
    let _skTemp0 = findMSB(_globalUniforms.a);
    (*_stageOut).sk_FragColor.x = f32(_skTemp0);
    let _skTemp1 = findMSB(_globalUniforms.b);
    (*_stageOut).sk_FragColor.y = f32(_skTemp1);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
