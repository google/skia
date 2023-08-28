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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var i1: i32 = 0;
    i1 = i1 + i32(1);
    var i2: i32 = 4660;
    i2 = i2 + i32(1);
    var i3: i32 = 32766;
    i3 = i3 + i32(1);
    var i4: i32 = -32766;
    i4 = i4 + i32(1);
    var i5: i32 = 19132;
    i5 = i5 + i32(1);
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
