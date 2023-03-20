cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float _10_testArray[5] : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float one = _10_testArray[0];
    float two = _10_testArray[1];
    float three = _10_testArray[2];
    float four = _10_testArray[3];
    float five = _10_testArray[4];
    bool _66 = false;
    if (mad(_10_testArray[0], _10_testArray[1], _10_testArray[2]) == 5.0f)
    {
        _66 = mad(_10_testArray[2], _10_testArray[3], _10_testArray[4]) == 17.0f;
    }
    else
    {
        _66 = false;
    }
    float4 _67 = 0.0f.xxxx;
    if (_66)
    {
        _67 = _10_colorGreen;
    }
    else
    {
        _67 = _10_colorRed;
    }
    return _67;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
