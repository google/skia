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

int out_param_func2_ih(out float _27)
{
    _27 = _12_colorRed.x;
    return int(_12_colorRed.x);
}

float4 main(float2 _36)
{
    float testArray[2] = { 0.0f, 0.0f };
    float _44 = 0.0f;
    int _45 = out_param_func2_ih(_44);
    testArray[0] = _44;
    bool _60 = false;
    if (testArray[0] == 1.0f)
    {
        _60 = testArray[1] == 1.0f;
    }
    else
    {
        _60 = false;
    }
    float4 _61 = 0.0f.xxxx;
    if (_60)
    {
        _61 = _12_colorGreen;
    }
    else
    {
        _61 = _12_colorRed;
    }
    return _61;
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
