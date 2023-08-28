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
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool;
    {
      var a: bool;
      const ONE: i32 = 1;
      var b: i32;
      var c: i32;
      let _skTemp0 = i32(_globalUniforms.colorGreen.y);
      switch _skTemp0 {
        case 0, 1, 2, 3, 4, 5 {
          var _skTemp1: bool = false;
          if _skTemp0 == 0 {
            ;
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 1 {
            ;
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 2 {
            b = ONE;
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 3 {
            {
              var d: f32 = f32(b);
              c = i32(d);
            }
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 4 {
            a = bool(c);
            // fallthrough
          }
          ok = a;
        }
        case default {}
      }
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
