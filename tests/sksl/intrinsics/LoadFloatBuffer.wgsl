diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct FloatBuffer {
  floatData: array<f32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : FloatBuffer;
fn avoidInline_vf(f: ptr<function, f32>) {
  {
    (*f) = _storage0.floatData[0];
  }
}
fn _skslMain() -> vec4<f32> {
  {
    var f: f32 = 0.0;
    var _skTemp1: f32;
    avoidInline_vf(&_skTemp1);
    f = _skTemp1;
    return vec4<f32>(f32(f));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
