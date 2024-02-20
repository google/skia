diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: vec4<f32> = vec4<f32>(1.0);
    loop {
      if x.w == 1.0 {
        {
          x.x = x.x - 0.25;
          if x.x <= 0.0 {
            break;
          }
        }
      } else {
        break;
      }
    }
    loop {
      if x.z > 0.0 {
        {
          x.z = x.z - 0.25;
          if x.w == 1.0 {
            continue;
          }
          x.y = 0.0;
        }
      } else {
        break;
      }
    }
    return x;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
