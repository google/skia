diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var u1: u32 = 0u;
    u1 = u1 + u32(1);
    var u2: u32 = 305441741u;
    u2 = u2 + u32(1);
    var u3: u32 = 2147483646u;
    u3 = u3 + u32(1);
    var u4: u32 = 4294967294u;
    u4 = u4 + u32(1);
    var u5: u32 = 65534u;
    u5 = u5 + u32(1);
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
