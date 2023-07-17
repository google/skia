diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn out_param_func1_vh(_skParam0: ptr<function, f32>) {
  let v = _skParam0;
  {
    (*v) = _globalUniforms.colorGreen.y;
  }
}
fn out_param_func2_ih(_skParam0: ptr<function, f32>) -> i32 {
  let v = _skParam0;
  {
    (*v) = _globalUniforms.colorRed.x;
    return i32((*v));
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let c = _skParam0;
  {
    var testArray: array<f32, 2>;
    var _skTemp0: f32;
    let _skTemp1 = out_param_func2_ih(&_skTemp0);
    testArray[0] = _skTemp0;
    let _skTemp2 = _skTemp1;
    var _skTemp3: f32 = testArray[_skTemp2];
    out_param_func1_vh(&_skTemp3);
    testArray[_skTemp2] = _skTemp3;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((testArray[0] == 1.0) && (testArray[1] == 1.0)));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
