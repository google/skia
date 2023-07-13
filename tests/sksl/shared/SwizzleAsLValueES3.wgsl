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
    let _skTemp6 = Z_i();
    let _skTemp7 = _skTemp6;
    _array[_skTemp5].y = _array[_skTemp7].y * 4.0;
    let _skTemp8 = Z_i();
    let _skTemp9 = _skTemp8;
    let _skTemp10 = Z_i();
    let _skTemp11 = _skTemp10;
    _array[_skTemp9] = vec4<f32>((_array[_skTemp11].yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), _array[_skTemp9].x).wxyz;
    let _skTemp12 = Z_i();
    let _skTemp13 = _skTemp12;
    let _skTemp14 = Z_i();
    let _skTemp15 = _skTemp14;
    _array[_skTemp13] = (_array[_skTemp15].zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    let _skTemp16 = Z_i();
    let _skTemp17 = _skTemp16;
    let _skTemp18 = Z_i();
    let _skTemp19 = _skTemp18;
    var _skTemp20: f32;
    let _skTemp21 = Z_i();
    let _skTemp22 = _skTemp21;
    if _array[_skTemp22].w <= 1.0 {
      let _skTemp23 = Z_i();
      let _skTemp24 = _skTemp23;
      _skTemp20 = _array[_skTemp24].z;
    } else {
      let _skTemp25 = Z_i();
      _skTemp20 = f32(_skTemp25);
    }
    _array[_skTemp17].x = _array[_skTemp19].x + _skTemp20;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((gAccessCount == 8) && all(_array[0] == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
