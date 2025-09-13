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

float foo_ff2(float2 _27)
{
    return _27.x * _27.y;
}

void bar_vf(inout float _36)
{
    float y[2] = { 0.0f, 0.0f };
    y[0] = _36;
    y[1] = _36 * 2.0f;
    float2 _55 = float2(y[0], y[1]);
    _36 = foo_ff2(_55);
}

float4 main(float2 _58)
{
    float x = 10.0f;
    float _62 = 10.0f;
    bar_vf(_62);
    x = _62;
    float4 _68 = 0.0f.xxxx;
    if (_62 == 200.0f)
    {
        _68 = _13_colorGreen;
    }
    else
    {
        _68 = _13_colorRed;
    }
    return _68;
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
