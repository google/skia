cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _14_colorGreen : packoffset(c0);
    float4 _14_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float x = 0.0f;

bool out_params_are_distinct_from_global_bh(out float _29)
{
    _29 = 2.0f;
    bool _38 = false;
    if (x == 1.0f)
    {
        _38 = true;
    }
    else
    {
        _38 = false;
    }
    return _38;
}

float4 main(float2 _40)
{
    x = 1.0f;
    float _42 = 0.0f;
    bool _43 = out_params_are_distinct_from_global_bh(_42);
    x = _42;
    float4 _45 = 0.0f.xxxx;
    if (_43)
    {
        _45 = _14_colorGreen;
    }
    else
    {
        _45 = _14_colorRed;
    }
    return _45;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    float4 _26 = main(_24);
    sk_FragColor = _26;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
