diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    const b: bool = true;
    let s: i32 = i32(_globalUniforms.unknownInput);
    let i: i32 = i32(_globalUniforms.unknownInput);
    let us: u32 = u32(_globalUniforms.unknownInput);
    let ui: u32 = u32(_globalUniforms.unknownInput);
    let h: f16 = f16(_globalUniforms.unknownInput);
    let f: f32 = _globalUniforms.unknownInput;
    let s2s: i32 = s;
    let i2s: i32 = i32(i);
    let us2s: i32 = i32(us);
    let ui2s: i32 = i32(ui);
    let h2s: i32 = i32(h);
    let f2s: i32 = i32(f);
    let b2s: i32 = i32(b);
    let s2i: i32 = i32(s);
    let i2i: i32 = i;
    let us2i: i32 = i32(us);
    let ui2i: i32 = i32(ui);
    let h2i: i32 = i32(h);
    let f2i: i32 = i32(f);
    let b2i: i32 = i32(b);
    let s2us: u32 = u32(s);
    let i2us: u32 = u32(i);
    let us2us: u32 = us;
    let ui2us: u32 = u32(ui);
    let h2us: u32 = u32(h);
    let f2us: u32 = u32(f);
    let b2us: u32 = u32(b);
    let s2ui: u32 = u32(s);
    let i2ui: u32 = u32(i);
    let us2ui: u32 = u32(us);
    let ui2ui: u32 = ui;
    let h2ui: u32 = u32(h);
    let f2ui: u32 = u32(f);
    let b2ui: u32 = u32(b);
    let s2f: f32 = f32(s);
    let i2f: f32 = f32(i);
    let us2f: f32 = f32(us);
    let ui2f: f32 = f32(ui);
    let h2f: f32 = f32(h);
    let f2f: f32 = f;
    let b2f: f32 = f32(b);
    (*_stageOut).sk_FragColor.x = (((((((((((((((((((((f16(s) + f16(i)) + f16(us)) + f16(ui)) + h) + f16(f)) + f16(s2s)) + f16(i2s)) + f16(us2s)) + f16(ui2s)) + f16(h2s)) + f16(f2s)) + f16(b2s)) + f16(s2i)) + f16(i2i)) + f16(us2i)) + f16(ui2i)) + f16(h2i)) + f16(f2i)) + f16(b2i)) + f16(s2us)) + f16(i2us)) + f16(us2us);
    (*_stageOut).sk_FragColor.x = (*_stageOut).sk_FragColor.x + (((((((((((((((((f16(ui2us) + f16(h2us)) + f16(f2us)) + f16(b2us)) + f16(s2ui)) + f16(i2ui)) + f16(us2ui)) + f16(ui2ui)) + f16(h2ui)) + f16(f2ui)) + f16(b2ui)) + f16(s2f)) + f16(i2f)) + f16(us2f)) + f16(ui2f)) + f16(h2f)) + f16(f2f)) + f16(b2f));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
