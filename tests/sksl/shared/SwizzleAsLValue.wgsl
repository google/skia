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
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var scalar: vec4<f32>;
    var R_array: array<vec4<f32>, 1>;
    scalar = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    scalar.w = 2.0;
    scalar.y = scalar.y * 4.0;
    scalar = vec4<f32>((scalar.yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), scalar.x).wxyz;
    scalar = (scalar.zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    scalar.x = scalar.x + (select(0.0, scalar.z, scalar.w <= 1.0));
    R_array[0] = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    R_array[0].w = 2.0;
    R_array[0].y = R_array[0].y * 4.0;
    R_array[0] = vec4<f32>((R_array[0].yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), R_array[0].x).wxyz;
    R_array[0] = (R_array[0].zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    R_array[0].x = R_array[0].x + (select(0.0, R_array[0].z, R_array[0].w <= 1.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(scalar == vec4<f32>(1.0, 1.0, 0.25, 1.0)) && all(R_array[0] == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
