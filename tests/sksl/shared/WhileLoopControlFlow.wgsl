diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
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
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
