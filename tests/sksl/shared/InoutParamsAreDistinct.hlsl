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

bool inout_params_are_distinct_bhh(out float _28, out float _29)
{
    _28 = 1.0f;
    _29 = 2.0f;
    bool _37 = false;
    if (true)
    {
        _37 = true;
    }
    else
    {
        _37 = false;
    }
    return _37;
}

float4 main(float2 _39)
{
    float x = 0.0f;
    float _42 = 0.0f;
    float _43 = 0.0f;
    bool _44 = inout_params_are_distinct_bhh(_42, _43);
    x = _42;
    x = _43;
    float4 _47 = 0.0f.xxxx;
    if (_44)
    {
        _47 = _12_colorGreen;
    }
    else
    {
        _47 = _12_colorRed;
    }
    return _47;
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
