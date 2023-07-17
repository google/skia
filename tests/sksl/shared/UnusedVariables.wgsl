diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn userfunc_ff(_skParam0: f32) -> f32 {
  let v = _skParam0;
  {
    return v + 1.0;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var b: f32 = 2.0;
    var c: f32 = 3.0;
    b = 2.0;
    b = c + 77.0;
    let _skTemp0 = sin(c + 77.0);
    b = _skTemp0;
    let _skTemp1 = userfunc_ff(c + 77.0);
    let _skTemp2 = userfunc_ff(c + 77.0);
    b = _skTemp2;
    let _skTemp3 = cos(c);
    b = _skTemp3;
    b = b;
    {
      var x: i32 = 0;
      loop {
        {
          continue;
        }
        continuing {
          x = x + i32(1);
          break if x >= 1;
        }
      }
    }
    var d: f32 = c;
    b = 3.0;
    d = d + f32(1);
    return vec4<f32>(f32(b == 2.0), f32(b == 3.0), f32(d == 5.0), f32(d == 4.0));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
