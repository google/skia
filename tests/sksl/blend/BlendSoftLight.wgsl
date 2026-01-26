diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  src: vec4<f16>,
  dst: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const sk_PrivkGuardedDivideEpsilon: f16 = f16(select(0.0, 1e-08, false));
fn soft_light_component_Qhh2h2(s: vec2<f16>, d: vec2<f16>) -> f16 {
  {
    if (2.0h * s.x) <= s.y {
      {
        return (((d.x * d.x) * (s.y - 2.0h * s.x)) / (d.y + sk_PrivkGuardedDivideEpsilon) + (1.0h - d.y) * s.x) + d.x * ((-s.y + 2.0h * s.x) + 1.0h);
      }
    } else {
      if (4.0h * d.x) <= d.y {
        {
          let DSqd: f16 = d.x * d.x;
          let DCub: f16 = DSqd * d.x;
          let DaSqd: f16 = d.y * d.y;
          let DaCub: f16 = DaSqd * d.y;
          return (((DaSqd * (s.x - d.x * ((3.0h * s.y - 6.0h * s.x) - 1.0h)) + ((12.0h * d.y) * DSqd) * (s.y - 2.0h * s.x)) - (16.0h * DCub) * (s.y - 2.0h * s.x)) - DaCub * s.x) / (DaSqd + sk_PrivkGuardedDivideEpsilon);
        }
      } else {
        {
          return ((d.x * ((s.y - 2.0h * s.x) + 1.0h) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0h * s.x)) - d.y * s.x;
        }
      }
    }
  }
  return f16();
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var _skTemp0: vec4<f16>;
    if _globalUniforms.dst.w == 0.0h {
      _skTemp0 = _globalUniforms.src;
    } else {
      _skTemp0 = vec4<f16>(soft_light_component_Qhh2h2(_globalUniforms.src.xw, _globalUniforms.dst.xw), soft_light_component_Qhh2h2(_globalUniforms.src.yw, _globalUniforms.dst.yw), soft_light_component_Qhh2h2(_globalUniforms.src.zw, _globalUniforms.dst.zw), _globalUniforms.src.w + (1.0h - _globalUniforms.src.w) * _globalUniforms.dst.w);
    }
    (*_stageOut).sk_FragColor = _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
