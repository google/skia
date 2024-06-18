diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn IsEqual_bh4h4(x: vec4<f32>, y: vec4<f32>) -> bool {
  {
    return all(x == y);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var colorBlue: vec4<f32> = vec4<f32>(0.0, 0.0, _globalUniforms.colorWhite.zw);
    var colorGreen: vec4<f32> = vec4<f32>(0.0, _globalUniforms.colorWhite.y, 0.0, _globalUniforms.colorWhite.w);
    var colorRed: vec4<f32> = vec4<f32>(_globalUniforms.colorWhite.x, 0.0, 0.0, _globalUniforms.colorWhite.w);
    var _skTemp0: vec4<f32>;
    let _skTemp1 = IsEqual_bh4h4(_globalUniforms.colorWhite, colorBlue);
    if !_skTemp1 {
      var _skTemp2: vec4<f32>;
      let _skTemp3 = IsEqual_bh4h4(colorGreen, colorRed);
      if _skTemp3 {
        _skTemp2 = colorRed;
      } else {
        _skTemp2 = colorGreen;
      }
      _skTemp0 = _skTemp2;
    } else {
      var _skTemp4: vec4<f32>;
      let _skTemp5 = IsEqual_bh4h4(colorRed, colorGreen);
      if !_skTemp5 {
        _skTemp4 = colorBlue;
      } else {
        _skTemp4 = _globalUniforms.colorWhite;
      }
      _skTemp0 = _skTemp4;
    }
    var result: vec4<f32> = _skTemp0;
    var _skTemp6: vec4<f32>;
    let _skTemp7 = IsEqual_bh4h4(colorRed, colorBlue);
    if _skTemp7 {
      _skTemp6 = _globalUniforms.colorWhite;
    } else {
      var _skTemp8: vec4<f32>;
      let _skTemp9 = IsEqual_bh4h4(colorRed, colorGreen);
      if !_skTemp9 {
        _skTemp8 = result;
      } else {
        var _skTemp10: vec4<f32>;
        let _skTemp11 = IsEqual_bh4h4(colorRed, _globalUniforms.colorWhite);
        if _skTemp11 {
          _skTemp10 = colorBlue;
        } else {
          _skTemp10 = colorRed;
        }
        _skTemp8 = _skTemp10;
      }
      _skTemp6 = _skTemp8;
    }
    return _skTemp6;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
