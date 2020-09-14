
precision mediump float;
precision mediump sampler2D;
in mediump vec2 x;
mediump vec4 main() {
    mediump vec2 _2_InlineA;
    {
        mediump vec2 _3_reusedName = x + vec2(1.0, 2.0);
        mediump vec2 _4_0_InlineB;
        {
            mediump vec2 _5_1_reusedName = _3_reusedName + vec2(3.0, 4.0);
            _4_0_InlineB = _5_1_reusedName;
        }

        _2_InlineA = _4_0_InlineB;

    }

    return _2_InlineA.xyxy;

}
