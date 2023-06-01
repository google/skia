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
fn mat3x3f32_eq_mat3x3f32(left: mat3x3<f32>, right: mat3x3<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
fn mat4x4f32_eq_mat4x4f32(left: mat4x4<f32>, right: mat4x4<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
fn test3x3_b() -> bool {
    var matrix: mat3x3<f32>;
    var values: vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
    {
        var index: i32 = 0;
        loop {
            if index < 3 {
                {
                    matrix[index] = values;
                    values += 3.0;
                }
            } else {
                break;
            }
            continuing {
                let _skTemp0 = &(index);
                (*_skTemp0) += i32(1);
            }
        }
    }
    return mat3x3f32_eq_mat3x3f32(matrix, _globalUniforms.testMatrix3x3);
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
                    values += 4.0;
                }
            } else {
                break;
            }
            continuing {
                let _skTemp1 = &(index);
                (*_skTemp1) += i32(1);
            }
        }
    }
    return mat4x4f32_eq_mat4x4f32(matrix, _globalUniforms.testMatrix4x4);
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    if test3x3_b() {
        _skTemp3 = test4x4_b();
    } else {
        _skTemp3 = false;
    }
    if _skTemp3 {
        _skTemp2 = _globalUniforms.colorGreen;
    } else {
        _skTemp2 = _globalUniforms.colorRed;
    }
    return _skTemp2;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
