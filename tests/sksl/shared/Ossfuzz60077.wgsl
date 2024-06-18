diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
