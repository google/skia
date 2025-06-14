struct S
{
    int i;
};

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

float4 fnGreen_h4bf2(bool _29, float2 _30)
{
    return _13_colorGreen;
}

float4 fnRed_h4ifS(int _41, float _42, S _43)
{
    return _13_colorRed;
}

float4 main(float2 _49)
{
    float4 _55 = 0.0f.xxxx;
    if (_13_colorGreen.y != 0.0f)
    {
        bool _61 = true;
        float2 _63 = _49;
        _55 = fnGreen_h4bf2(_61, _63);
    }
    else
    {
        int _66 = 123;
        float _68 = 3.1400001049041748046875f;
        S _69 = { 0 };
        S _70 = _69;
        _55 = fnRed_h4ifS(_66, _68, _70);
    }
    return _55;
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
