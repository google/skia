diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
var<private> globalValue: i32 = 0;
fn side_effecting_ii(value: i32) -> i32 {
  {
    globalValue = globalValue + i32(1);
    return value;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _7_two: i32 = 2;
    const _8_flatten0: i32 = 1;
    var _9_flatten1: i32 = _7_two;
    const _10_flatten2: i32 = 3;
    _7_two = _7_two - i32(1);
    let _skTemp0 = side_effecting_ii(2);
    var _11_noFlatten0: i32 = array<i32, 3>(_7_two, _skTemp0, 3)[0];
    let _skTemp1 = side_effecting_ii(1);
    var _12_noFlatten1: i32 = array<i32, 3>(_skTemp1, 2, 3)[1];
    _7_two = _7_two + i32(1);
    var _13_noFlatten2: i32 = array<i32, 3>(1, _7_two, 3)[2];
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((_8_flatten0 == _11_noFlatten0) && (_9_flatten1 == _12_noFlatten1)) && (_10_flatten2 == _13_noFlatten2)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
