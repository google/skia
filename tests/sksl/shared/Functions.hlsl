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

float foo_ff2(float2 _23)
{
    return _23.x * _23.y;
}

void bar_vf(inout float _32)
{
    float y[2] = { 0.0f, 0.0f };
    y[0] = _32;
    y[1] = _32 * 2.0f;
    float2 _52 = float2(y[0], y[1]);
    _32 = foo_ff2(_52);
}

float4 main(float2 _55)
{
    float x = 10.0f;
    float _59 = 10.0f;
    bar_vf(_59);
    x = _59;
    float4 _65 = 0.0f.xxxx;
    if (_59 == 200.0f)
    {
        _65 = _9_colorGreen;
    }
    else
    {
        _65 = _9_colorRed;
    }
    return _65;
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
