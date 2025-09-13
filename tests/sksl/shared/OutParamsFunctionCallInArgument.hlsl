cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

int out_param_func2_ih(out float _36)
{
    _36 = _13_colorRed.x;
    return int(_13_colorRed.x);
}

void out_param_func1_vh(out float _28)
{
    _28 = _13_colorGreen.y;
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
    bool _70 = false;
    if (testArray[0] == 1.0f)
    {
        _70 = testArray[1] == 1.0f;
    }
    else
    {
        _70 = false;
    }
    float4 _71 = 0.0f.xxxx;
    if (_70)
    {
        _71 = _13_colorGreen;
    }
    else
    {
        _71 = _13_colorRed;
    }
    return _71;
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
