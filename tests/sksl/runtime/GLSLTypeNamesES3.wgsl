diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
var<private> f: f32;
var<private> u: u32;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let uv2: vec2<u32> = vec2<u32>(u);
    let uv3: vec3<u32> = vec3<u32>(uv2, u);
    let uv4: vec4<u32> = vec4<u32>(uv3, u);
    let _skTemp0 = mat2x2<f32>(f, 0.0, 0.0, f);
    let _skTemp1 = mat2x3<f32>(_skTemp0[0][0], _skTemp0[0][1], 0.0, _skTemp0[1][0], _skTemp0[1][1], 0.0);
    let m2: mat2x4<f32> = mat2x4<f32>(_skTemp1[0][0], _skTemp1[0][1], _skTemp1[0][2], 0.0, _skTemp1[1][0], _skTemp1[1][1], _skTemp1[1][2], 0.0);
    let _skTemp2 = mat3x2<f32>(f, 0.0, 0.0, f, 0.0, 0.0);
    let _skTemp3 = mat3x3<f32>(_skTemp2[0][0], _skTemp2[0][1], 0.0, _skTemp2[1][0], _skTemp2[1][1], 0.0, _skTemp2[2][0], _skTemp2[2][1], 1.0);
    let m3: mat3x4<f32> = mat3x4<f32>(_skTemp3[0][0], _skTemp3[0][1], _skTemp3[0][2], 0.0, _skTemp3[1][0], _skTemp3[1][1], _skTemp3[1][2], 0.0, _skTemp3[2][0], _skTemp3[2][1], _skTemp3[2][2], 0.0);
    let _skTemp4 = mat4x2<f32>(f, 0.0, 0.0, f, 0.0, 0.0, 0.0, 0.0);
    let _skTemp5 = mat4x3<f32>(_skTemp4[0][0], _skTemp4[0][1], 0.0, _skTemp4[1][0], _skTemp4[1][1], 0.0, _skTemp4[2][0], _skTemp4[2][1], 1.0, _skTemp4[3][0], _skTemp4[3][1], 0.0);
    let m4: mat4x4<f32> = mat4x4<f32>(_skTemp5[0][0], _skTemp5[0][1], _skTemp5[0][2], 0.0, _skTemp5[1][0], _skTemp5[1][1], _skTemp5[1][2], 0.0, _skTemp5[2][0], _skTemp5[2][1], _skTemp5[2][2], 0.0, _skTemp5[3][0], _skTemp5[3][1], _skTemp5[3][2], 1.0);
    return ((vec4<f32>(uv4) * m2[0]) * m3[0]) * m4[0];
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
