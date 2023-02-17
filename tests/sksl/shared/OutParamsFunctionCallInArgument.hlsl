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

int out_param_func2_ih(out float _36)
{
    _36 = _12_colorRed.x;
    return int(_12_colorRed.x);
}

void out_param_func1_vh(out float _27)
{
    _27 = _12_colorGreen.y;
}

float4 main(float2 _44)
{
    float testArray[2] = { 0.0f, 0.0f };
    float _51 = 0.0f;
    int _52 = out_param_func2_ih(_51);
    testArray[0] = _51;
    float _56 = testArray[_52];
    out_param_func1_vh(_56);
    testArray[_52] = _56;
    bool _69 = false;
    if (testArray[0] == 1.0f)
    {
        _69 = testArray[1] == 1.0f;
    }
    else
    {
        _69 = false;
    }
    float4 _70 = 0.0f.xxxx;
    if (_69)
    {
        _70 = _12_colorGreen;
    }
    else
    {
        _70 = _12_colorRed;
    }
    return _70;
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
