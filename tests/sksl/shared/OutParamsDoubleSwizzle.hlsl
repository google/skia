cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2 swizzle_lvalue_h2hhh2h(float _27, float _28, inout float2 _29, float _30)
{
    _29 = float2(_29.y, _29.x);
    return float2(_27 + _28, _30);
}

void func_vh4(inout float4 _42)
{
    float _46 = 1.0f;
    float _48 = 2.0f;
    float2 _51 = _42.xz;
    float _53 = 5.0f;
    float2 _54 = swizzle_lvalue_h2hhh2h(_46, _48, _51, _53);
    _42 = float4(_51.x, _42.y, _51.y, _42.w);
    float2 t = _54;
    _42 = float4(_42.x, _54.x, _42.z, _54.y);
}

float4 main(float2 _61)
{
    float4 result = float4(0.0f, 1.0f, 2.0f, 3.0f);
    float4 _66 = float4(0.0f, 1.0f, 2.0f, 3.0f);
    func_vh4(_66);
    result = _66;
    float4 _73 = 0.0f.xxxx;
    if (all(bool4(_66.x == float4(2.0f, 3.0f, 0.0f, 5.0f).x, _66.y == float4(2.0f, 3.0f, 0.0f, 5.0f).y, _66.z == float4(2.0f, 3.0f, 0.0f, 5.0f).z, _66.w == float4(2.0f, 3.0f, 0.0f, 5.0f).w)))
    {
        _73 = _12_colorGreen;
    }
    else
    {
        _73 = _12_colorRed;
    }
    return _73;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
