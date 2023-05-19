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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _outParamHelper_1_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn _outParamHelper_3_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn _outParamHelper_5_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn Increment_ii(y: ptr<function, i32>) -> i32 {
    (*y) += 1;
    return (*y);
}
fn TrueFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp0: bool;
    if x == 1 {
        _skTemp0 = true;
    } else {
        _skTemp0 = _outParamHelper_1_Increment_ii(&y) == 3;
    }
    if (_skTemp0) {
        {
            return x == 1 && y == 1;
        }
    } else {
        {
            return false;
        }
    }
}
fn FalseTrue_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp2: bool;
    if x == 2 {
        _skTemp2 = true;
    } else {
        _skTemp2 = _outParamHelper_3_Increment_ii(&y) == 2;
    }
    if (_skTemp2) {
        {
            return x == 1 && y == 2;
        }
    } else {
        {
            return false;
        }
    }
}
fn FalseFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp4: bool;
    if x == 2 {
        _skTemp4 = true;
    } else {
        _skTemp4 = _outParamHelper_5_Increment_ii(&y) == 3;
    }
    if (_skTemp4) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 2;
        }
    }
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    {
        _0_TrueTrue = _2_y == 1;
    }
    var _skTemp6: vec4<f32>;
    var _skTemp7: bool;
    var _skTemp8: bool;
    var _skTemp9: bool;
    if _0_TrueTrue {
        _skTemp9 = TrueFalse_b();
    } else {
        _skTemp9 = false;
    }
    if _skTemp9 {
        _skTemp8 = FalseTrue_b();
    } else {
        _skTemp8 = false;
    }
    if _skTemp8 {
        _skTemp7 = FalseFalse_b();
    } else {
        _skTemp7 = false;
    }
    if _skTemp7 {
        _skTemp6 = _globalUniforms.colorGreen;
    } else {
        _skTemp6 = _globalUniforms.colorRed;
    }
    return _skTemp6;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
