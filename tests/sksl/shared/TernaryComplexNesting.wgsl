diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn IsEqual_bh4h4(x: vec4<f32>, y: vec4<f32>) -> bool {
  {
    return all(x == y);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let colorBlue: vec4<f32> = vec4<f32>(0.0, 0.0, _globalUniforms.colorWhite.zw);
    let colorGreen: vec4<f32> = vec4<f32>(0.0, _globalUniforms.colorWhite.y, 0.0, _globalUniforms.colorWhite.w);
    let colorRed: vec4<f32> = vec4<f32>(_globalUniforms.colorWhite.x, 0.0, 0.0, _globalUniforms.colorWhite.w);
    var _skTemp0: vec4<f32>;
    if !IsEqual_bh4h4(_globalUniforms.colorWhite, colorBlue) {
      var _skTemp1: vec4<f32>;
      if IsEqual_bh4h4(colorGreen, colorRed) {
        _skTemp1 = colorRed;
      } else {
        _skTemp1 = colorGreen;
      }
      _skTemp0 = _skTemp1;
    } else {
      var _skTemp2: vec4<f32>;
      if !IsEqual_bh4h4(colorRed, colorGreen) {
        _skTemp2 = colorBlue;
      } else {
        _skTemp2 = _globalUniforms.colorWhite;
      }
      _skTemp0 = _skTemp2;
    }
    let result: vec4<f32> = _skTemp0;
    var _skTemp3: vec4<f32>;
    if IsEqual_bh4h4(colorRed, colorBlue) {
      _skTemp3 = _globalUniforms.colorWhite;
    } else {
      var _skTemp4: vec4<f32>;
      if !IsEqual_bh4h4(colorRed, colorGreen) {
        _skTemp4 = result;
      } else {
        var _skTemp5: vec4<f32>;
        if IsEqual_bh4h4(colorRed, _globalUniforms.colorWhite) {
          _skTemp5 = colorBlue;
        } else {
          _skTemp5 = colorRed;
        }
        _skTemp4 = _skTemp5;
      }
      _skTemp3 = _skTemp4;
    }
    return _skTemp3;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
