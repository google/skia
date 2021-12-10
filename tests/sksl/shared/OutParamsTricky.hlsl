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

float2 tricky_h2hhh2h(float _27, float _28, inout float2 _29, float _30)
{
    _29 = _29.yx;
    return float2(_27 + _28, _30);
}

void func_vh4(inout float4 _41)
{
    float _45 = 1.0f;
    float _47 = 2.0f;
    float2 _50 = _41.xz;
    float _52 = 5.0f;
    float2 _53 = tricky_h2hhh2h(_45, _47, _50, _52);
    _41 = float4(_50.x, _41.y, _50.y, _41.w);
    float2 t = _53;
    _41 = float4(_41.x, t.x, _41.z, t.y);
}

float4 main(float2 _61)
{
    float4 result = float4(0.0f, 1.0f, 2.0f, 3.0f);
    func_vh4(result);
    float4 _72 = 0.0f.xxxx;
    if (all(bool4(result.x == float4(2.0f, 3.0f, 0.0f, 5.0f).x, result.y == float4(2.0f, 3.0f, 0.0f, 5.0f).y, result.z == float4(2.0f, 3.0f, 0.0f, 5.0f).z, result.w == float4(2.0f, 3.0f, 0.0f, 5.0f).w)))
    {
        _72 = _12_colorGreen;
    }
    else
    {
        _72 = _12_colorRed;
    }
    return _72;
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
