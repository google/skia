
in vec2 x;
vec4 main() {
    vec2 _2_InlineA;
    vec2 _3_reusedName = x + vec2(1.0, 2.0);
    vec2 _4_InlineB;
    vec2 _5_reusedName = _3_reusedName + vec2(3.0, 4.0);
    _4_InlineB = _5_reusedName;

    _2_InlineA = _4_InlineB;


    return _2_InlineA.xyxy;

}
