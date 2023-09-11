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

int out_param_func2_ih(out float _33)
{
    _33 = _9_colorRed.x;
    return int(_9_colorRed.x);
}

void out_param_func1_vh(out float _24)
{
    _24 = _9_colorGreen.y;
}

float4 main(float2 _41)
{
    float testArray[2] = { 0.0f, 0.0f };
    float _48 = 0.0f;
    int _49 = out_param_func2_ih(_48);
    testArray[0] = _48;
    float _53 = testArray[_49];
    out_param_func1_vh(_53);
    testArray[_49] = _53;
    bool _67 = false;
    if (testArray[0] == 1.0f)
    {
        _67 = testArray[1] == 1.0f;
    }
    else
    {
        _67 = false;
    }
    float4 _68 = 0.0f.xxxx;
    if (_67)
    {
        _68 = _9_colorGreen;
    }
    else
    {
        _68 = _9_colorRed;
    }
    return _68;
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
