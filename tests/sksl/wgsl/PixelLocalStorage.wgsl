diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable chromium_experimental_pixel_local;
struct PixelLocalData {
  i: i32,
  f: f32,
};
var<pixel_local> pls: PixelLocalData;
fn _skslMain() {
  {
    pls.i = pls.i + i32(1);
    pls.f = pls.f * 2.0;
  }
}
@fragment fn main() {
  _skslMain();
}
