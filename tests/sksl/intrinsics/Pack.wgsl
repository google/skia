### Compilation failed:

error: :12:20 error: unresolved call target 'packHalf2x16'
    let _skTemp0 = packHalf2x16(vec2<f32>(_globalUniforms.a));
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: vec2<f32>,
  b: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = packHalf2x16(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp0);
    let _skTemp1 = packUnorm2x16(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp1);
    let _skTemp2 = packSnorm2x16(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp2);
    let _skTemp3 = packUnorm4x8(vec4<f32>(_globalUniforms.b));
    (*_stageOut).sk_FragColor.x = f32(_skTemp3);
    let _skTemp4 = packSnorm4x8(vec4<f32>(_globalUniforms.b));
    (*_stageOut).sk_FragColor.x = f32(_skTemp4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
