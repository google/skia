cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
    float4 _15_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int gAccessCount = 0;

int Z_i()
{
    gAccessCount++;
    return 0;
}

float4 main(float2 _34)
{
    gAccessCount = 0;
    int _44 = Z_i();
    float4 array[1] = { 0.0f.xxxx };
    array[_44] = _15_colorGreen * 0.5f;
    int _48 = Z_i();
    array[_48].w = 2.0f;
    int _53 = Z_i();
    array[_53].y *= 4.0f;
    int _59 = Z_i();
    float3 _69 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), array[_59].yzw);
    array[_59] = float4(array[_59].x, _69.x, _69.y, _69.z);
    int _72 = Z_i();
    float4 _79 = array[_72].zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    array[_72] = float4(_79.w, _79.y, _79.x, _79.z);
    int _82 = Z_i();
    int _86 = Z_i();
    float _92 = 0.0f;
    if (array[_86].w <= 1.0f)
    {
        int _96 = Z_i();
        _92 = array[_96].z;
    }
    else
    {
        int _100 = Z_i();
        _92 = float(_100);
    }
    array[_82].x += _92;
    bool _116 = false;
    if (gAccessCount == 8)
    {
        _116 = all(bool4(array[0].x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, array[0].y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, array[0].z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, array[0].w == float4(1.0f, 1.0f, 0.25f, 1.0f).w));
    }
    else
    {
        _116 = false;
    }
    float4 _117 = 0.0f.xxxx;
    if (_116)
    {
        _117 = _15_colorGreen;
    }
    else
    {
        _117 = _15_colorRed;
    }
    return _117;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    float4 _27 = main(_25);
    sk_FragColor = _27;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
