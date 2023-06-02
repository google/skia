cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool out_params_are_distinct_bhh(out float _26, out float _27)
{
    _26 = 1.0f;
    _27 = 2.0f;
    bool _35 = false;
    if (true)
    {
        _35 = true;
    }
    else
    {
        _35 = false;
    }
    return _35;
}

float4 main(float2 _37)
{
    float x = 0.0f;
    float _40 = 0.0f;
    float _41 = 0.0f;
    bool _42 = out_params_are_distinct_bhh(_40, _41);
    x = _40;
    x = _41;
    float4 _45 = 0.0f.xxxx;
    if (_42)
    {
        _45 = _11_colorGreen;
    }
    else
    {
        _45 = _11_colorRed;
    }
    return _45;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
