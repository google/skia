### Compilation failed:

error: :11:17 error: unresolved type 'sampler2D'
var<private> s: sampler2D;
                ^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorXform: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
var<private> s: sampler2D;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    var tmpColor: vec4<f32>;
    let _skTemp0 = sample(s, vec2<f32>(1.0));
    tmpColor = vec4<f32>(_skTemp0);
    let _skTemp1 = clamp((_globalUniforms.colorXform * vec4<f32>(tmpColor.xyz, 1.0)).xyz, vec3<f32>(0.0), vec3<f32>(tmpColor.w));
    let _skTemp2 = mat4x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    (*_stageOut).sk_FragColor = vec4<f32>(select(tmpColor, vec4<f32>(_skTemp1, tmpColor.w), vec4<bool>((any(_globalUniforms.colorXform[0] != _skTemp2[0]) || any(_globalUniforms.colorXform[1] != _skTemp2[1]) || any(_globalUniforms.colorXform[2] != _skTemp2[2]) || any(_globalUniforms.colorXform[3] != _skTemp2[3])))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
