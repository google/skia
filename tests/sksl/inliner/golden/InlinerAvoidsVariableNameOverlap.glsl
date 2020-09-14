
precision mediump float;
precision mediump sampler2D;
in mediump vec2 x;
mediump vec4 main() {
    mediump vec2 _1_InlineA;
    {
        mediump vec2 reusedName = x + vec2(1.0, 2.0);
        mediump vec2 _0_InlineB;
        {
            mediump vec2 reusedName = reusedName + vec2(3.0, 4.0);
            _0_InlineB = reusedName;
        }

        _1_InlineA = _0_InlineB;

    }

    return _1_InlineA.xyxy;

}
