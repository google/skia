### Compilation failed:

error: :18:20 error: sqrt must be called with a value >= 0
    let _skTemp0 = sqrt(negativeVal);
                   ^^^^^^^^^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  var coords = _skParam0;
  {
    const negativeVal: vec4<f32> = vec4<f32>(-1.0, -4.0, -16.0, -64.0);
    let _skTemp0 = sqrt(negativeVal);
    coords = _skTemp0.xy;
    var inputVal: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) + vec4<f32>(0.0, 2.0, 6.0, 12.0);
    const expected: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    const allowedDelta: vec4<f32> = vec4<f32>(0.05);
    let _skTemp1 = sqrt(inputVal.x);
    let _skTemp2 = abs(_skTemp1 - 1.0);
    let _skTemp3 = sqrt(inputVal.xy);
    let _skTemp4 = abs(_skTemp3 - vec2<f32>(1.0, 2.0));
    let _skTemp5 = all(_skTemp4 < vec2<f32>(0.05));
    let _skTemp6 = sqrt(inputVal.xyz);
    let _skTemp7 = abs(_skTemp6 - vec3<f32>(1.0, 2.0, 3.0));
    let _skTemp8 = all(_skTemp7 < vec3<f32>(0.05));
    let _skTemp9 = sqrt(inputVal);
    let _skTemp10 = abs(_skTemp9 - expected);
    let _skTemp11 = all(_skTemp10 < allowedDelta);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((_skTemp2 < 0.05 && _skTemp5) && _skTemp8) && _skTemp11));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
