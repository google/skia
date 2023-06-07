fn d_vi(_skParam0: ptr<function, i32>) {
  let i = _skParam0;
  {
    {
      var x: i32 = 0;
      loop {
        if x < 10 {
          (*i) = (*i) + i32(1);
        } else {
          break;
        }
        continuing {
          x = x + i32(1);
        }
      }
    }
  }
}
fn c_vi(_skParam0: ptr<function, i32>) {
  let i = _skParam0;
  {
    {
      var x: i32 = 0;
      loop {
        if x < 10 {
          var _skTemp0: i32 = (*i);
          d_vi(&_skTemp0);
          (*i) = _skTemp0;
        } else {
          break;
        }
        continuing {
          x = x + i32(1);
        }
      }
    }
  }
}
fn b_vi(_skParam0: ptr<function, i32>) {
  let i = _skParam0;
  {
    {
      var x: i32 = 0;
      loop {
        if x < 10 {
          var _skTemp1: i32 = (*i);
          c_vi(&_skTemp1);
          (*i) = _skTemp1;
        } else {
          break;
        }
        continuing {
          x = x + i32(1);
        }
      }
    }
  }
}
fn a_vi(_skParam0: ptr<function, i32>) {
  let i = _skParam0;
  {
    {
      var x: i32 = 0;
      loop {
        if x < 10 {
          var _skTemp2: i32 = (*i);
          b_vi(&_skTemp2);
          (*i) = _skTemp2;
        } else {
          break;
        }
        continuing {
          x = x + i32(1);
        }
      }
    }
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var i: i32 = 0;
    var _skTemp3: i32 = i;
    a_vi(&_skTemp3);
    i = _skTemp3;
    return vec4<f32>(0.0);
  }
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return main(_coords);
}
