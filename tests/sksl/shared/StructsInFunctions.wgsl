diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  x: f32,
  y: i32,
};
struct Nested {
  a: S,
  b: S,
};
struct Compound {
  f4: vec4<f32>,
  i3: vec3<i32>,
};
fn returns_a_struct_S() -> S {
  {
    var s: S;
    s.x = 1.0;
    s.y = 2;
    return s;
  }
}
fn constructs_a_struct_S() -> S {
  {
    return S(2.0, 3);
  }
}
fn accepts_a_struct_fS(s: S) -> f32 {
  {
    return s.x + f32(s.y);
  }
}
fn modifies_a_struct_vS(s: ptr<function, S>) {
  {
    (*s).x = (*s).x + f32(1);
    (*s).y = (*s).y + i32(1);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = returns_a_struct_S();
    var s: S = _skTemp0;
    let _skTemp1 = accepts_a_struct_fS(s);
    var x: f32 = _skTemp1;
    var _skTemp2: S = s;
    modifies_a_struct_vS(&_skTemp2);
    s = _skTemp2;
    let _skTemp3 = constructs_a_struct_S();
    var expected: S = _skTemp3;
    var n1: Nested;
    var n2: Nested;
    var n3: Nested;
    let _skTemp4 = returns_a_struct_S();
    n1.a = _skTemp4;
    n1.b = n1.a;
    n2 = n1;
    n3 = n2;
    var _skTemp5: S = n3.b;
    modifies_a_struct_vS(&_skTemp5);
    n3.b = _skTemp5;
    var c1: Compound = Compound(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    var c2: Compound = Compound(vec4<f32>(f32(_globalUniforms.colorGreen.y), 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    var c3: Compound = Compound(vec4<f32>(f32(_globalUniforms.colorGreen.x), 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    var _skTemp6: bool;
    let _skTemp7 = S(2.0, 3);
    if ((((x == 3.0) && (s.x == 2.0)) && (s.y == 3)) && ((s.x == expected.x) && (s.y == expected.y))) && ((s.x == _skTemp7.x) && (s.y == _skTemp7.y)) {
      let _skTemp8 = returns_a_struct_S();
      let _skTemp9 = _skTemp8;
      _skTemp6 = ((s.x != _skTemp9.x) || (s.y != _skTemp9.y));
    } else {
      _skTemp6 = false;
    }
    let _skTemp10 = Nested(S(1.0, 2), S(2.0, 3));
    var valid: bool = ((((_skTemp6 && (((n1.a.x == n2.a.x) && (n1.a.y == n2.a.y)) && ((n1.b.x == n2.b.x) && (n1.b.y == n2.b.y)))) && (((n1.a.x != n3.a.x) || (n1.a.y != n3.a.y)) || ((n1.b.x != n3.b.x) || (n1.b.y != n3.b.y)))) && (((n3.a.x == _skTemp10.a.x) && (n3.a.y == _skTemp10.a.y)) && ((n3.b.x == _skTemp10.b.x) && (n3.b.y == _skTemp10.b.y)))) && (all(c1.f4 == c2.f4) && all(c1.i3 == c2.i3))) && (any(c2.f4 != c3.f4) || any(c2.i3 != c3.i3));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(valid));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
