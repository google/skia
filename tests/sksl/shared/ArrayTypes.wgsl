diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct S {
  v: vec2<f32>,
};
fn initialize_vS(z: ptr<function, array<S, 2>>) {
  {
    (*z)[0].v = vec2<f32>(0.0, 1.0);
    (*z)[1].v = vec2<f32>(2.0, 1.0);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: array<vec2<f32>, 2>;
    x[0] = vec2<f32>(0.0);
    x[1] = vec2<f32>(1.0, 0.0);
    var y: array<vec2<f32>, 2>;
    y[0] = vec2<f32>(0.0, 1.0);
    y[1] = vec2<f32>(-1.0, 2.0);
    var z: array<S, 2>;
    var _skTemp0: array<S, 2>;
    initialize_vS(&_skTemp0);
    z = _skTemp0;
    return vec4<f32>(f32(x[0].x * x[0].y + z[0].v.x), f32(x[1].x - x[1].y * z[0].v.y), f32((y[0].x / y[0].y) / z[1].v.x), f32(y[1].x + y[1].y * z[1].v.y));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
