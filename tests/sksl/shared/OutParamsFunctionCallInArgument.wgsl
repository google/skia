diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn out_param_func1_vh(v: ptr<function, f16>) {
  {
    (*v) = _globalUniforms.colorGreen.y;
  }
}
fn out_param_func2_ih(v: ptr<function, f16>) -> i32 {
  {
    (*v) = _globalUniforms.colorRed.x;
    return i32((*v));
  }
}
fn _skslMain(c: vec2<f32>) -> vec4<f16> {
  {
    var testArray: array<f16, 2>;
    var _skTemp0: f16;
    let _skTemp1 = out_param_func2_ih(&_skTemp0);
    testArray[0] = _skTemp0;
    let _skTemp2 = _skTemp1;
    var _skTemp3: f16 = testArray[_skTemp2];
    out_param_func1_vh(&_skTemp3);
    testArray[_skTemp2] = _skTemp3;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((testArray[0] == 1.0h) && (testArray[1] == 1.0h)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
