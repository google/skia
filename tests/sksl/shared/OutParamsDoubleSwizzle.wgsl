diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn swizzle_lvalue_h2hhh2h(x: f16, y: f16, color: ptr<function, vec2<f16>>, z: f16) -> vec2<f16> {
  {
    (*color) = ((*color)).yx;
    return vec2<f16>(x + y, z);
  }
}
fn func_vh4(color: ptr<function, vec4<f16>>) {
  {
    var _skTemp0: vec2<f16> = (*color).xz;
    let _skTemp1 = swizzle_lvalue_h2hhh2h(1.0h, 2.0h, &_skTemp0, 5.0h);
    (*color) = vec4<f16>((_skTemp0), (*color).yw).xzyw;
    let t: vec2<f16> = _skTemp1;
    (*color) = vec4<f16>((*color).xz, (t)).xzyw;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var result: vec4<f16> = vec4<f16>(0.0h, 1.0h, 2.0h, 3.0h);
    var _skTemp2: vec4<f16> = result;
    func_vh4(&_skTemp2);
    result = _skTemp2;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(result == vec4<f16>(2.0h, 3.0h, 0.0h, 5.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
