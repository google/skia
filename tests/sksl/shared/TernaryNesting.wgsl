diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var colorBlue: vec4<f32> = vec4<f32>(0.0, 0.0, _globalUniforms.colorWhite.zw);
    var colorGreen: vec4<f32> = vec4<f32>(0.0, _globalUniforms.colorWhite.y, 0.0, _globalUniforms.colorWhite.w);
    var colorRed: vec4<f32> = vec4<f32>(_globalUniforms.colorWhite.x, 0.0, 0.0, _globalUniforms.colorWhite.w);
    var result: vec4<f32> = select(select(_globalUniforms.colorWhite, colorBlue, vec4<bool>(any(colorRed != colorGreen))), select(colorGreen, colorRed, vec4<bool>(all(colorGreen == colorRed))), vec4<bool>(any(_globalUniforms.colorWhite != colorBlue)));
    return select(select(select(colorRed, colorBlue, vec4<bool>(all(colorRed == _globalUniforms.colorWhite))), result, vec4<bool>(any(colorRed != colorGreen))), _globalUniforms.colorWhite, vec4<bool>(all(colorRed == colorBlue)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
