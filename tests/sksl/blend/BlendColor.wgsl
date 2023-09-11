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
fn blend_color_saturation_Qhh3(color: vec3<f32>) -> f32 {
  {
    let _skTemp0 = max(color.x, color.y);
    let _skTemp1 = max(_skTemp0, color.z);
    let _skTemp2 = min(color.x, color.y);
    let _skTemp3 = min(_skTemp2, color.z);
    return _skTemp1 - _skTemp3;
  }
}
fn blend_hslc_h4h2h4h4(flipSat: vec2<f32>, src: vec4<f32>, dst: vec4<f32>) -> vec4<f32> {
  {
    var alpha: f32 = dst.w * src.w;
    var sda: vec3<f32> = src.xyz * dst.w;
    var dsa: vec3<f32> = dst.xyz * src.w;
    var l: vec3<f32> = select(sda, dsa, vec3<bool>(bool(flipSat.x)));
    var r: vec3<f32> = select(dsa, sda, vec3<bool>(bool(flipSat.x)));
    if bool(flipSat.y) {
      {
        let _skTemp4 = min(l.x, l.y);
        let _skTemp5 = min(_skTemp4, l.z);
        var _2_mn: f32 = _skTemp5;
        let _skTemp6 = max(l.x, l.y);
        let _skTemp7 = max(_skTemp6, l.z);
        var _3_mx: f32 = _skTemp7;
        let _skTemp8 = blend_color_saturation_Qhh3(r);
        l = select(vec3<f32>(0.0), ((l - _2_mn) * _skTemp8) / (_3_mx - _2_mn), vec3<bool>(_3_mx > _2_mn));
        r = dsa;
      }
    }
    let _skTemp9 = dot(vec3<f32>(0.3, 0.59, 0.11), r);
    var _4_lum: f32 = _skTemp9;
    let _skTemp10 = dot(vec3<f32>(0.3, 0.59, 0.11), l);
    var _5_result: vec3<f32> = (_4_lum - _skTemp10) + l;
    let _skTemp11 = min(_5_result.x, _5_result.y);
    let _skTemp12 = min(_skTemp11, _5_result.z);
    var _6_minComp: f32 = _skTemp12;
    let _skTemp13 = max(_5_result.x, _5_result.y);
    let _skTemp14 = max(_skTemp13, _5_result.z);
    var _7_maxComp: f32 = _skTemp14;
    if (_6_minComp < 0.0) && (_4_lum != _6_minComp) {
      {
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / ((_4_lum - _6_minComp) + sk_PrivkGuardedDivideEpsilon));
      }
    }
    if (_7_maxComp > alpha) && (_7_maxComp != _4_lum) {
      {
        _5_result = _4_lum + ((_5_result - _4_lum) * (alpha - _4_lum)) / ((_7_maxComp - _4_lum) + sk_PrivkGuardedDivideEpsilon);
      }
    }
    return vec4<f32>((((_5_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp15 = blend_hslc_h4h2h4h4(vec2<f32>(0.0), _globalUniforms.src, _globalUniforms.dst);
    (*_stageOut).sk_FragColor = _skTemp15;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
