diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const sk_PrivkGuardedDivideEpsilon: f32 = f32(select(0.0, 1e-08, false));
const sk_PrivkMinNormalHalf: f32 = 6.10351562e-05;
fn blend_color_saturation_Qhh3(color: vec3<f32>) -> f32 {
  {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
  }
}
fn blend_hslc_h4h2h4h4(flipSat: vec2<f32>, src: vec4<f32>, dst: vec4<f32>) -> vec4<f32> {
  {
    let alpha: f32 = dst.w * src.w;
    let sda: vec3<f32> = src.xyz * dst.w;
    let dsa: vec3<f32> = dst.xyz * src.w;
    var l: vec3<f32> = select(sda, dsa, vec3<bool>(bool(flipSat.x)));
    var r: vec3<f32> = select(dsa, sda, vec3<bool>(bool(flipSat.x)));
    if bool(flipSat.y) {
      {
        let _2_mn: f32 = min(min(l.x, l.y), l.z);
        let _3_mx: f32 = max(max(l.x, l.y), l.z);
        var _skTemp0: vec3<f32>;
        if _3_mx > _2_mn {
          _skTemp0 = ((l - _2_mn) * blend_color_saturation_Qhh3(r)) / (_3_mx - _2_mn);
        } else {
          _skTemp0 = vec3<f32>(0.0);
        }
        l = _skTemp0;
        r = dsa;
      }
    }
    let _4_lum: f32 = dot(vec3<f32>(0.3, 0.59, 0.11), r);
    var _5_result: vec3<f32> = (_4_lum - dot(vec3<f32>(0.3, 0.59, 0.11), l)) + l;
    let _6_minComp: f32 = min(min(_5_result.x, _5_result.y), _5_result.z);
    let _7_maxComp: f32 = max(max(_5_result.x, _5_result.y), _5_result.z);
    if (_6_minComp < 0.0) && (_4_lum != _6_minComp) {
      {
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / (((_4_lum - _6_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
      }
    }
    if (_7_maxComp > alpha) && (_7_maxComp != _4_lum) {
      {
        _5_result = _4_lum + ((_5_result - _4_lum) * (alpha - _4_lum)) / (((_7_maxComp - _4_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
      }
    }
    return vec4<f32>((((_5_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = blend_hslc_h4h2h4h4(vec2<f32>(0.0), _globalUniforms.src, _globalUniforms.dst);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
