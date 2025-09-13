cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2 swizzle_lvalue_h2hhh2h(float _28, float _29, inout float2 _30, float _31)
{
    _30 = float2(_30.y, _30.x);
    return float2(_28 + _29, _31);
}

void func_vh4(inout float4 _43)
{
    float _47 = 1.0f;
    float _49 = 2.0f;
    float2 _52 = _43.xz;
    float _54 = 5.0f;
    float2 _55 = swizzle_lvalue_h2hhh2h(_47, _49, _52, _54);
    _43 = float4(_52.x, _43.y, _52.y, _43.w);
    float2 t = _55;
    _43 = float4(_43.x, _55.x, _43.z, _55.y);
}

float4 main(float2 _62)
{
    float4 result = float4(0.0f, 1.0f, 2.0f, 3.0f);
    float4 _67 = float4(0.0f, 1.0f, 2.0f, 3.0f);
    func_vh4(_67);
    result = _67;
    float4 _75 = 0.0f.xxxx;
    if (all(bool4(_67.x == float4(2.0f, 3.0f, 0.0f, 5.0f).x, _67.y == float4(2.0f, 3.0f, 0.0f, 5.0f).y, _67.z == float4(2.0f, 3.0f, 0.0f, 5.0f).z, _67.w == float4(2.0f, 3.0f, 0.0f, 5.0f).w)))
    {
        _75 = _13_colorGreen;
    }
    else
    {
        _75 = _13_colorRed;
    }
    return _75;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
