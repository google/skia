struct S
{
    int i;
};

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

float4 fnGreen_h4bf2(bool _25, float2 _26)
{
    return _9_colorGreen;
}

float4 fnRed_h4ifS(int _38, float _39, S _40)
{
    return _9_colorRed;
}

float4 main(float2 _46)
{
    float4 _52 = 0.0f.xxxx;
    if (_9_colorGreen.y != 0.0f)
    {
        bool _58 = true;
        float2 _60 = _46;
        _52 = fnGreen_h4bf2(_58, _60);
    }
    else
    {
        int _63 = 123;
        float _65 = 3.1400001049041748046875f;
        S _66 = { 0 };
        S _67 = _66;
        _52 = fnRed_h4ifS(_63, _65, _67);
    }
    return _52;
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
