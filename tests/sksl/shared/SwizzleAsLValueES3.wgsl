diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
var<private> gAccessCount: i32 = 0;
fn Z_i() -> i32 {
  {
    gAccessCount = gAccessCount + i32(1);
    return 0;
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var R_array: array<vec4<f32>, 1>;
    let _skTemp0 = Z_i();
    R_array[_skTemp0] = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    let _skTemp1 = Z_i();
    R_array[_skTemp1].w = 2.0;
    let _skTemp2 = Z_i();
    R_array[_skTemp2].y = R_array[_skTemp2].y * 4.0;
    let _skTemp3 = Z_i();
    R_array[_skTemp3] = vec4<f32>(R_array[_skTemp3].x, (R_array[_skTemp3].yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)));
    let _skTemp4 = Z_i();
    R_array[_skTemp4] = (R_array[_skTemp4].zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    let _skTemp5 = Z_i();
    var _skTemp6: f32;
    let _skTemp7 = Z_i();
    if R_array[_skTemp7].w <= 1.0 {
      let _skTemp8 = Z_i();
      _skTemp6 = R_array[_skTemp8].z;
    } else {
      _skTemp6 = f32(Z_i());
    }
    R_array[_skTemp5].x = R_array[_skTemp5].x + _skTemp6;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((gAccessCount == 8) && all(R_array[0] == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
