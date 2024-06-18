diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var dest: texture_storage_2d<rgba32float, write>;
fn _skslMain(_stageIn: CSIn) {
  {
    var pixel: vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    var max_x: f32 = 5.0;
    var max_y: f32 = 5.0;
    let _skTemp0 = textureDimensions(dest);
    let _skTemp1 = textureDimensions(dest);
    var x: f32 = f32(_stageIn.sk_GlobalInvocationID.x * 2u - _skTemp0.x) / f32(_skTemp1.x);
    let _skTemp2 = textureDimensions(dest);
    let _skTemp3 = textureDimensions(dest);
    var y: f32 = f32(_stageIn.sk_GlobalInvocationID.y * 2u - _skTemp2.y) / f32(_skTemp3.y);
    var ray_origin: vec3<f32> = vec3<f32>(0.0, 0.0, -1.0);
    var ray_target: vec3<f32> = vec3<f32>(x * max_x, y * max_y, 0.0);
    var sphere_center: vec3<f32> = vec3<f32>(0.0, 0.0, -10.0);
    var sphere_radius: f32 = 1.0;
    var t_minus_c: vec3<f32> = ray_target - sphere_center;
    let _skTemp4 = dot(ray_origin, t_minus_c);
    var b: f32 = _skTemp4;
    let _skTemp5 = dot(t_minus_c, t_minus_c);
    var c: f32 = _skTemp5 - sphere_radius * sphere_radius;
    var bsqmc: f32 = b * b - c;
    if bsqmc >= 0.0 {
      {
        pixel = vec4<f32>(0.4, 0.4, 1.0, 1.0);
      }
    }
    textureStore(dest, _stageIn.sk_GlobalInvocationID.xy, pixel);
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
