cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float _11_testArray[5] : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float one = _11_testArray[0];
    float two = _11_testArray[1];
    float three = _11_testArray[2];
    float four = _11_testArray[3];
    float five = _11_testArray[4];
    bool _67 = false;
    if (mad(_11_testArray[0], _11_testArray[1], _11_testArray[2]) == 5.0f)
    {
        _67 = mad(_11_testArray[2], _11_testArray[3], _11_testArray[4]) == 17.0f;
    }
    else
    {
        _67 = false;
    }
    float4 _68 = 0.0f.xxxx;
    if (_67)
    {
        _68 = _11_colorGreen;
    }
    else
    {
        _68 = _11_colorRed;
    }
    return _68;
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
