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
    if _globalUniforms.unknownInput > 5.0 {
      {
        (*_stageOut).sk_FragColor = vec4<f16>(0.75h);
      }
    } else {
      {
        discard;
      }
    }
    var i: i32 = 0;
    loop {
      if i < 10 {
        {
          (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor * 0.5h;
          i = i + i32(1);
        }
      } else {
        break;
      }
    }
    loop {
      {
        (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor + 0.25h;
      }
      continuing {
        break if (*_stageOut).sk_FragColor.x >= 0.75h;
      }
    }
    {
      var i: i32 = 0;
      loop {
        {
          if (i % 2) == 1 {
            break;
          } else {
            if i > 100 {
              return ;
            } else {
              continue;
            }
          }
        }
        continuing {
          i = i + i32(1);
          break if i >= 10;
        }
      }
    }
    return ;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
