diagnostic(off, derivative_uniformity);
struct FSIn {
  @location(1) input1: f32,
  @location(2) input2: f32,
  @location(3) input3: vec2<f32>,
  @location(4) @interpolate(flat) input4: i32,
  @location(5) @interpolate(flat) input5: vec2<i32>,
};
struct FSOut {
  @location(1) output1: f32,
  @location(2) output2: f32,
  @location(3) output3: vec2<f32>,
  @location(4) @interpolate(flat) output4: i32,
  @location(5) @interpolate(flat) output5: vec2<i32>,
};
fn _skslMain() {
  {
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain();
  return _stageOut;
}
