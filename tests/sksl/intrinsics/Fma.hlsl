cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float _7_testArray[5] : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float one = _7_testArray[0];
    float two = _7_testArray[1];
    float three = _7_testArray[2];
    float four = _7_testArray[3];
    float five = _7_testArray[4];
    bool _64 = false;
    if (mad(_7_testArray[0], _7_testArray[1], _7_testArray[2]) == 5.0f)
    {
        _64 = mad(_7_testArray[2], _7_testArray[3], _7_testArray[4]) == 17.0f;
    }
    else
    {
        _64 = false;
    }
    float4 _65 = 0.0f.xxxx;
    if (_64)
    {
        _65 = _7_colorGreen;
    }
    else
    {
        _65 = _7_colorRed;
    }
    return _65;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
