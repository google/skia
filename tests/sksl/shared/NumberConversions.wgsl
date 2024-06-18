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
    var b: bool = true;
    var s: i32 = i32(_globalUniforms.unknownInput);
    var i: i32 = i32(_globalUniforms.unknownInput);
    var us: u32 = u32(_globalUniforms.unknownInput);
    var ui: u32 = u32(_globalUniforms.unknownInput);
    var h: f32 = f32(_globalUniforms.unknownInput);
    var f: f32 = _globalUniforms.unknownInput;
    var s2s: i32 = s;
    var i2s: i32 = i32(i);
    var us2s: i32 = i32(us);
    var ui2s: i32 = i32(ui);
    var h2s: i32 = i32(h);
    var f2s: i32 = i32(f);
    var b2s: i32 = i32(b);
    var s2i: i32 = i32(s);
    var i2i: i32 = i;
    var us2i: i32 = i32(us);
    var ui2i: i32 = i32(ui);
    var h2i: i32 = i32(h);
    var f2i: i32 = i32(f);
    var b2i: i32 = i32(b);
    var s2us: u32 = u32(s);
    var i2us: u32 = u32(i);
    var us2us: u32 = us;
    var ui2us: u32 = u32(ui);
    var h2us: u32 = u32(h);
    var f2us: u32 = u32(f);
    var b2us: u32 = u32(b);
    var s2ui: u32 = u32(s);
    var i2ui: u32 = u32(i);
    var us2ui: u32 = u32(us);
    var ui2ui: u32 = ui;
    var h2ui: u32 = u32(h);
    var f2ui: u32 = u32(f);
    var b2ui: u32 = u32(b);
    var s2f: f32 = f32(s);
    var i2f: f32 = f32(i);
    var us2f: f32 = f32(us);
    var ui2f: f32 = f32(ui);
    var h2f: f32 = f32(h);
    var f2f: f32 = f;
    var b2f: f32 = f32(b);
    (*_stageOut).sk_FragColor.x = (((((((((((((((((((((f32(s) + f32(i)) + f32(us)) + f32(ui)) + h) + f32(f)) + f32(s2s)) + f32(i2s)) + f32(us2s)) + f32(ui2s)) + f32(h2s)) + f32(f2s)) + f32(b2s)) + f32(s2i)) + f32(i2i)) + f32(us2i)) + f32(ui2i)) + f32(h2i)) + f32(f2i)) + f32(b2i)) + f32(s2us)) + f32(i2us)) + f32(us2us);
    (*_stageOut).sk_FragColor.x = (*_stageOut).sk_FragColor.x + (((((((((((((((((f32(ui2us) + f32(h2us)) + f32(f2us)) + f32(b2us)) + f32(s2ui)) + f32(i2ui)) + f32(us2ui)) + f32(ui2ui)) + f32(h2ui)) + f32(f2ui)) + f32(b2ui)) + f32(s2f)) + f32(i2f)) + f32(us2f)) + f32(ui2f)) + f32(h2f)) + f32(f2f)) + f32(b2f));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
