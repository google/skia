var<private> gInitialized: f32;
var<private> gInitializedFromOther: f32;
var<private> gUninitialized: f32;
fn init_globals_v() {
    gUninitialized = 1.0;
}
fn main(xy: vec2<f32>) -> vec4<f32> {
    init_globals_v();
    return vec4<f32>(0.0, gInitializedFromOther, 0.0, gUninitialized);
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
