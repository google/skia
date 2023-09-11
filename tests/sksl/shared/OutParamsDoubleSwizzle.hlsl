cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2 swizzle_lvalue_h2hhh2h(float _24, float _25, inout float2 _26, float _27)
{
    _26 = float2(_26.y, _26.x);
    return float2(_24 + _25, _27);
}

void func_vh4(inout float4 _39)
{
    float _43 = 1.0f;
    float _45 = 2.0f;
    float2 _48 = _39.xz;
    float _50 = 5.0f;
    float2 _51 = swizzle_lvalue_h2hhh2h(_43, _45, _48, _50);
    _39 = float4(_48.x, _39.y, _48.y, _39.w);
    float2 t = _51;
    _39 = float4(_39.x, _51.x, _39.z, _51.y);
}

float4 main(float2 _58)
{
    float4 result = float4(0.0f, 1.0f, 2.0f, 3.0f);
    float4 _63 = float4(0.0f, 1.0f, 2.0f, 3.0f);
    func_vh4(_63);
    result = _63;
    float4 _71 = 0.0f.xxxx;
    if (all(bool4(_63.x == float4(2.0f, 3.0f, 0.0f, 5.0f).x, _63.y == float4(2.0f, 3.0f, 0.0f, 5.0f).y, _63.z == float4(2.0f, 3.0f, 0.0f, 5.0f).z, _63.w == float4(2.0f, 3.0f, 0.0f, 5.0f).w)))
    {
        _71 = _9_colorGreen;
    }
    else
    {
        _71 = _9_colorRed;
    }
    return _71;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
