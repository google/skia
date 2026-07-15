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
const sk_PrivkHalfEpsilon: f16 = 0.000244140625h;
const sk_PrivkMinNormalHalf: f16 = 6.10351562e-05h;
fn blend_color_saturation_Qhh3(color: vec3<f16>) -> f16 {
  {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
  }
}
fn guarded_divide_Qh3h3h(n: vec3<f16>, d: f16) -> vec3<f16> {
  {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
  }
}
fn blend_hslc_h4h2h4h4(flipSat: vec2<f16>, src: vec4<f16>, dst: vec4<f16>) -> vec4<f16> {
  {
    let alpha: f16 = dst.w * src.w;
    let sda: vec3<f16> = src.xyz * dst.w;
    let dsa: vec3<f16> = dst.xyz * src.w;
    var l: vec3<f16> = select(sda, dsa, vec3<bool>(bool(flipSat.x)));
    var r: vec3<f16> = select(dsa, sda, vec3<bool>(bool(flipSat.x)));
    if bool(flipSat.y) {
      {
        let _2_mn: f16 = min(min(l.x, l.y), l.z);
        let _3_mx: f16 = max(max(l.x, l.y), l.z);
        let _4_diff: f16 = _3_mx - _2_mn;
        var _skTemp0: vec3<f16>;
        if _4_diff >= sk_PrivkHalfEpsilon {
          _skTemp0 = guarded_divide_Qh3h3h((l - _2_mn) * blend_color_saturation_Qhh3(r), _4_diff);
        } else {
          _skTemp0 = vec3<f16>(0.0h);
        }
        l = _skTemp0;
        r = dsa;
      }
    }
    let _5_lum: f16 = dot(vec3<f16>(0.3h, 0.59h, 0.11h), r);
    var _6_result: vec3<f16> = (_5_lum - dot(vec3<f16>(0.3h, 0.59h, 0.11h), l)) + l;
    let _7_minComp: f16 = min(min(_6_result.x, _6_result.y), _6_result.z);
    let _8_maxComp: f16 = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0h) && (_5_lum != _7_minComp) {
      {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
      }
    }
    if (_8_maxComp > alpha) && (_8_maxComp != _5_lum) {
      {
        _6_result = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
      }
    }
    return vec4<f16>((((_6_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = blend_hslc_h4h2h4h4(vec2<f16>(0.0h), _globalUniforms.src, _globalUniforms.dst);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
