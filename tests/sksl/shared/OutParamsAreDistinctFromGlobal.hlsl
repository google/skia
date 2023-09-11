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

static float x = 0.0f;

bool out_params_are_distinct_from_global_bh(out float _27)
{
    _27 = 2.0f;
    bool _36 = false;
    if (x == 1.0f)
    {
        _36 = true;
    }
    else
    {
        _36 = false;
    }
    return _36;
}

float4 main(float2 _38)
{
    x = 1.0f;
    float _40 = 0.0f;
    bool _41 = out_params_are_distinct_from_global_bh(_40);
    x = _40;
    float4 _43 = 0.0f.xxxx;
    if (_41)
    {
        _43 = _11_colorGreen;
    }
    else
    {
        _43 = _11_colorRed;
    }
    return _43;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    float4 _23 = main(_21);
    sk_FragColor = _23;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
