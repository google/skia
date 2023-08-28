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
  colorBlack: vec4<f32>,
  colorWhite: vec4<f32>,
  testInputs: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var intGreen: vec4<i32> = vec4<i32>(_globalUniforms.colorGreen * 100.0);
    var intRed: vec4<i32> = vec4<i32>(_globalUniforms.colorRed * 100.0);
    let _skTemp0 = select(intGreen.x, intRed.x, false);
    let _skTemp1 = select(intGreen.xy, intRed.xy, vec2<bool>(false));
    let _skTemp2 = select(intGreen.xyz, intRed.xyz, vec3<bool>(false));
    let _skTemp3 = select(intGreen, intRed, vec4<bool>(false));
    let _skTemp4 = select(intGreen.x, intRed.x, true);
    let _skTemp5 = select(intGreen.xy, intRed.xy, vec2<bool>(true));
    let _skTemp6 = select(intGreen.xyz, intRed.xyz, vec3<bool>(true));
    let _skTemp7 = select(intGreen, intRed, vec4<bool>(true));
    let _skTemp8 = select(_globalUniforms.colorGreen.x, _globalUniforms.colorRed.x, false);
    let _skTemp9 = select(_globalUniforms.colorGreen.xy, _globalUniforms.colorRed.xy, vec2<bool>(false));
    let _skTemp10 = select(_globalUniforms.colorGreen.xyz, _globalUniforms.colorRed.xyz, vec3<bool>(false));
    let _skTemp11 = select(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<bool>(false));
    let _skTemp12 = select(_globalUniforms.colorGreen.x, _globalUniforms.colorRed.x, true);
    let _skTemp13 = select(_globalUniforms.colorGreen.xy, _globalUniforms.colorRed.xy, vec2<bool>(true));
    let _skTemp14 = select(_globalUniforms.colorGreen.xyz, _globalUniforms.colorRed.xyz, vec3<bool>(true));
    let _skTemp15 = select(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<bool>(true));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((((((((((((((_skTemp0 == intGreen.x) && all(_skTemp1 == intGreen.xy)) && all(_skTemp2 == intGreen.xyz)) && all(_skTemp3 == intGreen)) && (_skTemp4 == intRed.x)) && all(_skTemp5 == intRed.xy)) && all(_skTemp6 == intRed.xyz)) && all(_skTemp7 == intRed)) && (0 == intGreen.x)) && all(vec2<i32>(0, 100) == intGreen.xy)) && all(vec3<i32>(0, 100, 0) == intGreen.xyz)) && all(vec4<i32>(0, 100, 0, 100) == intGreen)) && (100 == intRed.x)) && all(vec2<i32>(100, 0) == intRed.xy)) && all(vec3<i32>(100, 0, 0) == intRed.xyz)) && all(vec4<i32>(100, 0, 0, 100) == intRed)) && (_skTemp8 == _globalUniforms.colorGreen.x)) && all(_skTemp9 == _globalUniforms.colorGreen.xy)) && all(_skTemp10 == _globalUniforms.colorGreen.xyz)) && all(_skTemp11 == _globalUniforms.colorGreen)) && (_skTemp12 == _globalUniforms.colorRed.x)) && all(_skTemp13 == _globalUniforms.colorRed.xy)) && all(_skTemp14 == _globalUniforms.colorRed.xyz)) && all(_skTemp15 == _globalUniforms.colorRed)) && (0.0 == _globalUniforms.colorGreen.x)) && all(vec2<f32>(0.0, 1.0) == _globalUniforms.colorGreen.xy)) && all(vec3<f32>(0.0, 1.0, 0.0) == _globalUniforms.colorGreen.xyz)) && all(vec4<f32>(0.0, 1.0, 0.0, 1.0) == _globalUniforms.colorGreen)) && (1.0 == _globalUniforms.colorRed.x)) && all(vec2<f32>(1.0, 0.0) == _globalUniforms.colorRed.xy)) && all(vec3<f32>(1.0, 0.0, 0.0) == _globalUniforms.colorRed.xyz)) && all(vec4<f32>(1.0, 0.0, 0.0, 1.0) == _globalUniforms.colorRed)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
