var<private> f: f32;
fn main(coords: vec2<f32>) -> vec4<f32> {
    var fv: vec4<f32> = vec4<f32>(f);
    return fv;
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
