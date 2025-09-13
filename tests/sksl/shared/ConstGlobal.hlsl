cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _22_colorGreen : packoffset(c0);
    float4 _22_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4x4 MATRIXFIVE = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);

bool verify_const_globals_biih44(int _38, int _39, float4x4 _40)
{
    bool _51 = false;
    if (_38 == 7)
    {
        _51 = _39 == 10;
    }
    else
    {
        _51 = false;
    }
    bool _71 = false;
    if (_51)
    {
        _71 = ((all(bool4(_40[0].x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, _40[0].y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, _40[0].z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, _40[0].w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_40[1].x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, _40[1].y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, _40[1].z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, _40[1].w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(_40[2].x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, _40[2].y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, _40[2].z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, _40[2].w == float4(0.0f, 0.0f, 5.0f, 0.0f).w))) && all(bool4(_40[3].x == float4(0.0f, 0.0f, 0.0f, 5.0f).x, _40[3].y == float4(0.0f, 0.0f, 0.0f, 5.0f).y, _40[3].z == float4(0.0f, 0.0f, 0.0f, 5.0f).z, _40[3].w == float4(0.0f, 0.0f, 0.0f, 5.0f).w));
    }
    else
    {
        _71 = false;
    }
    return _71;
}

float4 main(float2 _73)
{
    MATRIXFIVE = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    int _75 = 7;
    int _76 = 10;
    float4x4 _77 = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    float4 _79 = 0.0f.xxxx;
    if (verify_const_globals_biih44(_75, _76, _77))
    {
        _79 = _22_colorGreen;
    }
    else
    {
        _79 = _22_colorRed;
    }
    return _79;
}

void frag_main()
{
    float2 _31 = 0.0f.xx;
    float4 _33 = main(_31);
    sk_FragColor = _33;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
