fn main(xy: vec2<f32>) -> vec4<f32> {
    var i: i32 = 0;
    return vec4<f32>(f32(i));
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
