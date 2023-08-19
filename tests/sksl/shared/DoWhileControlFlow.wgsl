diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var x: vec4<f32> = vec4<f32>(1.0);
    loop {
      {
        x.x = x.x - 0.25;
        if x.x <= 0.0 {
          break;
        }
      }
      continuing {
        break if x.w != 1.0;
      }
    }
    loop {
      {
        x.z = x.z - 0.25;
        if x.w == 1.0 {
          continue;
        }
        x.y = 0.0;
      }
      continuing {
        break if x.z <= 0.0;
      }
    }
    return x;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
