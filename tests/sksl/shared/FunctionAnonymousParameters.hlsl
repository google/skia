struct S
{
    int i;
};

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

float4 fnGreen_h4bf2(bool _27, float2 _28)
{
    return _12_colorGreen;
}

float4 fnRed_h4ifS(int _40, float _41, S _42)
{
    return _12_colorRed;
}

float4 main(float2 _48)
{
    float4 _54 = 0.0f.xxxx;
    if (_12_colorGreen.y != 0.0f)
    {
        bool _60 = true;
        float2 _62 = _48;
        _54 = fnGreen_h4bf2(_60, _62);
    }
    else
    {
        int _65 = 123;
        float _67 = 3.1400001049041748046875f;
        S _68 = { 0 };
        S _69 = _68;
        _54 = fnRed_h4ifS(_65, _67, _69);
    }
    return _54;
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
