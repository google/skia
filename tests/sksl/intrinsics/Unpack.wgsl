### Compilation failed:

error: :11:20 error: unresolved call target 'unpackHalf2x16'
    let _skTemp0 = unpackHalf2x16(_globalUniforms.a);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: u32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = unpackHalf2x16(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp0)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp1 = unpackUnorm2x16(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp1)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp2 = unpackSnorm2x16(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp2)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp3 = unpackUnorm4x8(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp3);
    let _skTemp4 = unpackSnorm4x8(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
