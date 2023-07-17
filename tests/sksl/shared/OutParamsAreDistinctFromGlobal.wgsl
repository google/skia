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
var<private> x: f32 = 1.0;
fn out_params_are_distinct_from_global_bh(_skParam0: ptr<function, f32>) -> bool {
  let y = _skParam0;
  {
    (*y) = 2.0;
    return (x == 1.0) && ((*y) == 2.0);
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: f32;
    let _skTemp2 = out_params_are_distinct_from_global_bh(&_skTemp1);
    x = _skTemp1;
    if _skTemp2 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
