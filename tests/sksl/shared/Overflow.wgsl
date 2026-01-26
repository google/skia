diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const h: f16 = 65503.9h;
    let _skTemp0 = h;
    let hugeH: f16 = (((((((((((((_skTemp0 * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h;
    const f: f32 = 1e+09;
    let _skTemp1 = 1e+36;
    let hugeF: f32 = ((((((((((_skTemp1 * f) * f) * f) * f) * f) * f) * f) * f) * f) * f) * f;
    let _skTemp2 = 1073741824;
    let hugeI: i32 = i32((((((((((((((((((((_skTemp2 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp3 = 2147483648u;
    let hugeU: u32 = ((((((((((((((((((_skTemp3 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp4 = 16384;
    let hugeS: i32 = ((((((((((((((((_skTemp4 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    let _skTemp5 = 32768u;
    let hugeUS: u32 = (((((((((((((((_skTemp5 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp6 = -2147483648;
    let hugeNI: i32 = i32(((((((((((((((((((_skTemp6 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp7 = -32768;
    let hugeNS: i32 = (((((((((((((((_skTemp7 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const i4: vec4<i32> = vec4<i32>(2);
    let _skTemp8 = vec4<i32>(1073741824);
    let hugeIvec: vec4<i32> = ((((((((((((((_skTemp8 * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const u4: vec4<u32> = vec4<u32>(2u);
    let _skTemp9 = vec4<u32>(2147483648u);
    let hugeUvec: vec4<u32> = (((((((((((((_skTemp9 * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    let _skTemp10 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let hugeMxM: mat4x4<f32> = _skTemp10 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let _skTemp11 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let hugeMxV: vec4<f32> = _skTemp11 * vec4<f32>(1e+20);
    let _skTemp12 = vec4<f32>(1e+20);
    let hugeVxM: vec4<f32> = _skTemp12 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    return ((((((((((((_globalUniforms.colorGreen * saturate(hugeH)) * saturate(f16(hugeF))) * saturate(f16(hugeI))) * saturate(f16(hugeU))) * saturate(f16(hugeS))) * saturate(f16(hugeUS))) * saturate(f16(hugeNI))) * saturate(f16(hugeNS))) * saturate(vec4<f16>(hugeIvec))) * saturate(vec4<f16>(hugeUvec))) * saturate(vec4<f16>(hugeMxM[0]))) * saturate(vec4<f16>(hugeMxV))) * saturate(vec4<f16>(hugeVxM));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
