diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    const b: bool = true;
    let s: i32 = i32(_globalUniforms.unknownInput);
    let i: i32 = i32(_globalUniforms.unknownInput);
    let us: u32 = u32(_globalUniforms.unknownInput);
    let ui: u32 = u32(_globalUniforms.unknownInput);
    let h: f32 = f32(_globalUniforms.unknownInput);
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
    (*_stageOut).sk_FragColor.x = (((((((((((((((((((((f32(s) + f32(i)) + f32(us)) + f32(ui)) + h) + f32(f)) + f32(s2s)) + f32(i2s)) + f32(us2s)) + f32(ui2s)) + f32(h2s)) + f32(f2s)) + f32(b2s)) + f32(s2i)) + f32(i2i)) + f32(us2i)) + f32(ui2i)) + f32(h2i)) + f32(f2i)) + f32(b2i)) + f32(s2us)) + f32(i2us)) + f32(us2us);
    (*_stageOut).sk_FragColor.x = (*_stageOut).sk_FragColor.x + (((((((((((((((((f32(ui2us) + f32(h2us)) + f32(f2us)) + f32(b2us)) + f32(s2ui)) + f32(i2ui)) + f32(us2ui)) + f32(ui2ui)) + f32(h2ui)) + f32(f2ui)) + f32(b2ui)) + f32(s2f)) + f32(i2f)) + f32(us2f)) + f32(ui2f)) + f32(h2f)) + f32(f2f)) + f32(b2f));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
