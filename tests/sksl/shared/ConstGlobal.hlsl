cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _21_colorGreen : packoffset(c0);
    float4 _21_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4x4 MATRIXFIVE = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);

bool verify_const_globals_biih44(int _37, int _38, float4x4 _39)
{
    bool _50 = false;
    if (_37 == 7)
    {
        _50 = _38 == 10;
    }
    else
    {
        _50 = false;
    }
    bool _70 = false;
    if (_50)
    {
        _70 = ((all(bool4(_39[0].x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, _39[0].y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, _39[0].z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, _39[0].w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_39[1].x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, _39[1].y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, _39[1].z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, _39[1].w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(_39[2].x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, _39[2].y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, _39[2].z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, _39[2].w == float4(0.0f, 0.0f, 5.0f, 0.0f).w))) && all(bool4(_39[3].x == float4(0.0f, 0.0f, 0.0f, 5.0f).x, _39[3].y == float4(0.0f, 0.0f, 0.0f, 5.0f).y, _39[3].z == float4(0.0f, 0.0f, 0.0f, 5.0f).z, _39[3].w == float4(0.0f, 0.0f, 0.0f, 5.0f).w));
    }
    else
    {
        _70 = false;
    }
    return _70;
}

float4 main(float2 _72)
{
    MATRIXFIVE = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    int _74 = 7;
    int _75 = 10;
    float4x4 _76 = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    float4 _78 = 0.0f.xxxx;
    if (verify_const_globals_biih44(_74, _75, _76))
    {
        _78 = _21_colorGreen;
    }
    else
    {
        _78 = _21_colorRed;
    }
    return _78;
}

void frag_main()
{
    float2 _30 = 0.0f.xx;
    float4 _32 = main(_30);
    sk_FragColor = _32;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
