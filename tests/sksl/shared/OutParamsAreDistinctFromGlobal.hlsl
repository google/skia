cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
    float4 _15_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float x = 0.0f;

bool out_params_are_distinct_from_global_bh(out float _31)
{
    _31 = 2.0f;
    bool _40 = false;
    if (x == 1.0f)
    {
        _40 = true;
    }
    else
    {
        _40 = false;
    }
    return _40;
}

float4 main(float2 _42)
{
    x = 1.0f;
    float _44 = 0.0f;
    bool _45 = out_params_are_distinct_from_global_bh(_44);
    x = _44;
    float4 _47 = 0.0f.xxxx;
    if (_45)
    {
        _47 = _15_colorGreen;
    }
    else
    {
        _47 = _15_colorRed;
    }
    return _47;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    float4 _27 = main(_25);
    sk_FragColor = _27;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
