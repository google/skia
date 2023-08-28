diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn d_vi(_skParam0: i32) {
  {
    var b: i32 = 4;
  }
}
fn c_vi(i: i32) {
  {
    d_vi(i);
  }
}
fn b_vi(i: i32) {
  {
    c_vi(i);
  }
}
fn a_vi(i: i32) {
  {
    b_vi(i);
    b_vi(i);
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var i: i32;
    a_vi(i);
    return vec4<f32>(0.0);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
