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

float foo_ff2(float2 _26)
{
    return _26.x * _26.y;
}

void bar_vf(inout float _35)
{
    float y[2] = { 0.0f, 0.0f };
    y[0] = _35;
    y[1] = _35 * 2.0f;
    float2 _55 = float2(y[0], y[1]);
    _35 = foo_ff2(_55);
}

float4 main(float2 _58)
{
    float x = 10.0f;
    float _62 = 10.0f;
    bar_vf(_62);
    x = _62;
    float4 _67 = 0.0f.xxxx;
    if (_62 == 200.0f)
    {
        _67 = _12_colorGreen;
    }
    else
    {
        _67 = _12_colorRed;
    }
    return _67;
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
