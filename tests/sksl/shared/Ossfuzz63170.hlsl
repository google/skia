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

int out_param_func2_ih(out float _24)
{
    _24 = _8_colorRed.x;
    return int(_8_colorRed.x);
}

float4 main(float2 _33)
{
    float testArray[2] = { 0.0f, 0.0f };
    float _41 = 0.0f;
    int _42 = out_param_func2_ih(_41);
    testArray[0] = _41;
    bool _57 = false;
    if (testArray[0] == 1.0f)
    {
        _57 = testArray[1] == 1.0f;
    }
    else
    {
        _57 = false;
    }
    float4 _58 = 0.0f.xxxx;
    if (_57)
    {
        _58 = _8_colorGreen;
    }
    else
    {
        _58 = _8_colorRed;
    }
    return _58;
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
