diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
fn d_vi(i: ptr<function, i32>) {
  {
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
    (*i) = (*i) + i32(1);
  }
}
fn c_vi(i: ptr<function, i32>) {
  {
    var _skTemp0: i32 = (*i);
    d_vi(&_skTemp0);
    (*i) = _skTemp0;
    var _skTemp1: i32 = (*i);
    d_vi(&_skTemp1);
    (*i) = _skTemp1;
    var _skTemp2: i32 = (*i);
    d_vi(&_skTemp2);
    (*i) = _skTemp2;
    var _skTemp3: i32 = (*i);
    d_vi(&_skTemp3);
    (*i) = _skTemp3;
    var _skTemp4: i32 = (*i);
    d_vi(&_skTemp4);
    (*i) = _skTemp4;
    var _skTemp5: i32 = (*i);
    d_vi(&_skTemp5);
    (*i) = _skTemp5;
    var _skTemp6: i32 = (*i);
    d_vi(&_skTemp6);
    (*i) = _skTemp6;
    var _skTemp7: i32 = (*i);
    d_vi(&_skTemp7);
    (*i) = _skTemp7;
    var _skTemp8: i32 = (*i);
    d_vi(&_skTemp8);
    (*i) = _skTemp8;
    var _skTemp9: i32 = (*i);
    d_vi(&_skTemp9);
    (*i) = _skTemp9;
  }
}
fn b_vi(i: ptr<function, i32>) {
  {
    var _skTemp10: i32 = (*i);
    c_vi(&_skTemp10);
    (*i) = _skTemp10;
    var _skTemp11: i32 = (*i);
    c_vi(&_skTemp11);
    (*i) = _skTemp11;
    var _skTemp12: i32 = (*i);
    c_vi(&_skTemp12);
    (*i) = _skTemp12;
    var _skTemp13: i32 = (*i);
    c_vi(&_skTemp13);
    (*i) = _skTemp13;
    var _skTemp14: i32 = (*i);
    c_vi(&_skTemp14);
    (*i) = _skTemp14;
    var _skTemp15: i32 = (*i);
    c_vi(&_skTemp15);
    (*i) = _skTemp15;
    var _skTemp16: i32 = (*i);
    c_vi(&_skTemp16);
    (*i) = _skTemp16;
    var _skTemp17: i32 = (*i);
    c_vi(&_skTemp17);
    (*i) = _skTemp17;
    var _skTemp18: i32 = (*i);
    c_vi(&_skTemp18);
    (*i) = _skTemp18;
    var _skTemp19: i32 = (*i);
    c_vi(&_skTemp19);
    (*i) = _skTemp19;
  }
}
fn a_vi(i: ptr<function, i32>) {
  {
    var _skTemp20: i32 = (*i);
    b_vi(&_skTemp20);
    (*i) = _skTemp20;
    var _skTemp21: i32 = (*i);
    b_vi(&_skTemp21);
    (*i) = _skTemp21;
    var _skTemp22: i32 = (*i);
    b_vi(&_skTemp22);
    (*i) = _skTemp22;
    var _skTemp23: i32 = (*i);
    b_vi(&_skTemp23);
    (*i) = _skTemp23;
    var _skTemp24: i32 = (*i);
    b_vi(&_skTemp24);
    (*i) = _skTemp24;
    var _skTemp25: i32 = (*i);
    b_vi(&_skTemp25);
    (*i) = _skTemp25;
    var _skTemp26: i32 = (*i);
    b_vi(&_skTemp26);
    (*i) = _skTemp26;
    var _skTemp27: i32 = (*i);
    b_vi(&_skTemp27);
    (*i) = _skTemp27;
    var _skTemp28: i32 = (*i);
    b_vi(&_skTemp28);
    (*i) = _skTemp28;
    var _skTemp29: i32 = (*i);
    b_vi(&_skTemp29);
    (*i) = _skTemp29;
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    var i: i32 = 0;
    var _skTemp30: i32 = i;
    a_vi(&_skTemp30);
    i = _skTemp30;
    return vec4<f32>(0.0);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
