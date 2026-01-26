diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
struct S {
  i: f16,
  j: f16,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var x: vec4<f16> = vec4<f16>(3.0h, 2.0h, 1.0h, 0.0h);
    x = vec4<f16>((x.zyx), x.w);
    var s: S;
    s.i = 2.0h;
    s.j = 2.0h;
    s.i = s.j;
    s.j = s.i;
    var a: array<f16, 2>;
    a[0] = 1.0h;
    a[1] = 0.0h;
    a[1] = a[0];
    return vec4<f16>(x.w, s.i / s.j, a[0] - a[1], a[0] * a[1]);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
