struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
    testMatrix3x3: mat3x3<f32>,
    testMatrix4x4: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test3x3_b() -> bool {
    var matrix: mat3x3<f32>;
    var values: vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
    {
        var index: i32 = 0;
        loop {
            if index < 3 {
                {
                    matrix[index] = values;
                    values = values + 3.0;
                }
            } else {
                break;
            }
            continuing {
                index = index + i32(1);
            }
        }
    }
    let _skTemp0 = matrix;
    let _skTemp1 = _globalUniforms.testMatrix3x3;
    return (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]) && all(_skTemp0[2] == _skTemp1[2]));
}
fn test4x4_b() -> bool {
    var matrix: mat4x4<f32>;
    var values: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    {
        var index: i32 = 0;
        loop {
            if index < 4 {
                {
                    matrix[index] = values;
                    values = values + 4.0;
                }
            } else {
                break;
            }
            continuing {
                index = index + i32(1);
            }
        }
    }
    let _skTemp2 = matrix;
    let _skTemp3 = _globalUniforms.testMatrix4x4;
    return (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]) && all(_skTemp2[2] == _skTemp3[2]) && all(_skTemp2[3] == _skTemp3[3]));
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _skTemp4: vec4<f32>;
    var _skTemp5: bool;
    let _skTemp6 = test3x3_b();
    if _skTemp6 {
        let _skTemp7 = test4x4_b();
        _skTemp5 = _skTemp7;
    } else {
        _skTemp5 = false;
    }
    if _skTemp5 {
        _skTemp4 = _globalUniforms.colorGreen;
    } else {
        _skTemp4 = _globalUniforms.colorRed;
    }
    return _skTemp4;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
