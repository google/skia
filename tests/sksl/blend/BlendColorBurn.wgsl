/*

:34:3 warning: code is unreachable
  return f32();
  ^^^^^^

*/

diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
const sk_PrivkGuardedDivideEpsilon: f32 = f32(select(0.0, 1e-08, false));
fn color_burn_component_Qhh2h2(s: vec2<f32>, d: vec2<f32>) -> f32 {
  {
    if d.y == d.x {
      {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
      }
    } else {
      if s.x == 0.0 {
        {
          return d.x * (1.0 - s.y);
        }
      } else {
        {
          let _skTemp0 = max(0.0, d.y - ((d.y - d.x) * s.y) / (s.x + sk_PrivkGuardedDivideEpsilon));
          var delta: f32 = _skTemp0;
          return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
      }
    }
  }
  return f32();
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp1 = color_burn_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw);
    let _skTemp2 = color_burn_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw);
    let _skTemp3 = color_burn_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp1, _skTemp2, _skTemp3, _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
