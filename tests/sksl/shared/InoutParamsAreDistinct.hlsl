cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool inout_params_are_distinct_bhh(out float _24, out float _25)
{
    _24 = 1.0f;
    _25 = 2.0f;
    bool _33 = false;
    if (true)
    {
        _33 = true;
    }
    else
    {
        _33 = false;
    }
    return _33;
}

float4 main(float2 _35)
{
    float x = 0.0f;
    float _38 = 0.0f;
    float _39 = 0.0f;
    bool _40 = inout_params_are_distinct_bhh(_38, _39);
    x = _38;
    x = _39;
    float4 _43 = 0.0f.xxxx;
    if (_40)
    {
        _43 = _8_colorGreen;
    }
    else
    {
        _43 = _8_colorRed;
    }
    return _43;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
