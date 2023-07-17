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
var<private> gAccessCount: i32 = 0;
fn Z_i() -> i32 {
  {
    gAccessCount = gAccessCount + i32(1);
    return 0;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var _array: array<vec4<f32>, 1>;
    let _skTemp0 = Z_i();
    let _skTemp1 = _skTemp0;
    _array[_skTemp1] = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    let _skTemp2 = Z_i();
    let _skTemp3 = _skTemp2;
    _array[_skTemp3].w = 2.0;
    let _skTemp4 = Z_i();
    let _skTemp5 = _skTemp4;
    _array[_skTemp5].y = _array[_skTemp5].y * 4.0;
    let _skTemp6 = Z_i();
    let _skTemp7 = _skTemp6;
    _array[_skTemp7] = vec4<f32>((_array[_skTemp7].yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), _array[_skTemp7].x).wxyz;
    let _skTemp8 = Z_i();
    let _skTemp9 = _skTemp8;
    _array[_skTemp9] = (_array[_skTemp9].zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    let _skTemp10 = Z_i();
    let _skTemp11 = _skTemp10;
    var _skTemp12: f32;
    let _skTemp13 = Z_i();
    let _skTemp14 = _skTemp13;
    if _array[_skTemp14].w <= 1.0 {
      let _skTemp15 = Z_i();
      let _skTemp16 = _skTemp15;
      _skTemp12 = _array[_skTemp16].z;
    } else {
      let _skTemp17 = Z_i();
      _skTemp12 = f32(_skTemp17);
    }
    _array[_skTemp11].x = _array[_skTemp11].x + _skTemp12;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((gAccessCount == 8) && all(_array[0] == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
