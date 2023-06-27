### Compilation failed:

error: :22:20 error: unresolved call target 'inverse'
    let _skTemp3 = inverse(mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


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
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var inv2x2: mat2x2<f32> = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    var inv3x3: mat3x3<f32> = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    var inv4x4: mat4x4<f32> = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    let _skTemp0 = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    let _skTemp1 = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    let _skTemp2 = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    let _skTemp3 = inverse(mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    let _skTemp4 = _skTemp3;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((all(_skTemp0[0] == inv2x2[0]) && all(_skTemp0[1] == inv2x2[1])) && (all(_skTemp1[0] == inv3x3[0]) && all(_skTemp1[1] == inv3x3[1]) && all(_skTemp1[2] == inv3x3[2]))) && (all(_skTemp2[0] == inv4x4[0]) && all(_skTemp2[1] == inv4x4[1]) && all(_skTemp2[2] == inv4x4[2]) && all(_skTemp2[3] == inv4x4[3]))) && (any(_skTemp4[0] != inv3x3[0]) || any(_skTemp4[1] != inv3x3[1]) || any(_skTemp4[2] != inv3x3[2]))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
