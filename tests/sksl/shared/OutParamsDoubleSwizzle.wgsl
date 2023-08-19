diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn swizzle_lvalue_h2hhh2h(_skParam0: f32, _skParam1: f32, _skParam2: ptr<function, vec2<f32>>, _skParam3: f32) -> vec2<f32> {
  let x = _skParam0;
  let y = _skParam1;
  let color = _skParam2;
  let z = _skParam3;
  {
    (*color) = ((*color)).yx;
    return vec2<f32>(x + y, z);
  }
}
fn func_vh4(_skParam0: ptr<function, vec4<f32>>) {
  let color = _skParam0;
  {
    var _skTemp0: vec2<f32> = (*color).xz;
    let _skTemp1 = swizzle_lvalue_h2hhh2h(1.0, 2.0, &_skTemp0, 5.0);
    (*color) = vec4<f32>((_skTemp0), (*color).yw).xzyw;
    var t: vec2<f32> = _skTemp1;
    (*color) = vec4<f32>((t), (*color).xz).zxwy;
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var result: vec4<f32> = vec4<f32>(0.0, 1.0, 2.0, 3.0);
    var _skTemp2: vec4<f32> = result;
    func_vh4(&_skTemp2);
    result = _skTemp2;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(result == vec4<f32>(2.0, 3.0, 0.0, 5.0))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
