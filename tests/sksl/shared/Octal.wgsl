diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var i1: i32 = 1;
    var i2: i32 = 342391;
    var i3: i32 = 2000000000;
    var i4: i32 = -2000000000;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((i1 == 1) && (i2 == 342391)) && (i3 == 2000000000)) && (i4 == -2000000000)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
