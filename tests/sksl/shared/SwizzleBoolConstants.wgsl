diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var v: vec4<bool> = vec4<bool>(bool(_globalUniforms.colorGreen.y));
    var result: vec4<bool> = vec4<bool>(v.x, true, true, true);
    result = vec4<bool>(v.xy, false, true);
    result = vec4<bool>(v.x, true, true, false);
    result = vec4<bool>(false, v.y, true, true);
    result = vec4<bool>(v.xyz, true);
    result = vec4<bool>(v.xy, true, true);
    result = vec4<bool>(v.x, false, v.z, true);
    result = vec4<bool>(v.x, true, false, false);
    result = vec4<bool>(true, v.yz, false);
    result = vec4<bool>(false, v.y, true, false);
    result = vec4<bool>(true, true, v.z, false);
    result = v;
    result = vec4<bool>(v.xyz, true);
    result = vec4<bool>(v.xy, false, v.w);
    result = vec4<bool>(v.xy, true, false);
    result = vec4<bool>(v.x, true, v.zw);
    result = vec4<bool>(v.x, false, v.z, true);
    result = vec4<bool>(v.x, true, true, v.w);
    result = vec4<bool>(v.x, true, false, true);
    result = vec4<bool>(true, v.yzw);
    result = vec4<bool>(false, v.yz, true);
    result = vec4<bool>(false, v.y, true, v.w);
    result = vec4<bool>(true, v.y, true, true);
    result = vec4<bool>(false, false, v.zw);
    result = vec4<bool>(false, false, v.z, true);
    result = vec4<bool>(false, true, true, v.w);
    let _skTemp0 = any(result);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
