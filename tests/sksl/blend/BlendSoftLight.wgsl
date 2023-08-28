/*

:37:3 warning: code is unreachable
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
fn soft_light_component_Qhh2h2(s: vec2<f32>, d: vec2<f32>) -> f32 {
  {
    if (2.0 * s.x) <= s.y {
      {
        return (((d.x * d.x) * (s.y - 2.0 * s.x)) / (d.y + sk_PrivkGuardedDivideEpsilon) + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
      }
    } else {
      if (4.0 * d.x) <= d.y {
        {
          var DSqd: f32 = d.x * d.x;
          var DCub: f32 = DSqd * d.x;
          var DaSqd: f32 = d.y * d.y;
          var DaCub: f32 = DaSqd * d.y;
          return (((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x) / (DaSqd + sk_PrivkGuardedDivideEpsilon);
        }
      } else {
        {
          let _skTemp0 = sqrt(d.y * d.x);
          return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - _skTemp0 * (s.y - 2.0 * s.x)) - d.y * s.x;
        }
      }
    }
  }
  return f32();
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp1 = soft_light_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw);
    let _skTemp2 = soft_light_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw);
    let _skTemp3 = soft_light_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw);
    (*_stageOut).sk_FragColor = select(vec4<f32>(_skTemp1, _skTemp2, _skTemp3, _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w), _globalUniforms.src, vec4<bool>(_globalUniforms.dst.w == 0.0));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
