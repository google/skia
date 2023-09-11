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

static int gAccessCount = 0;

int Z_i()
{
    gAccessCount++;
    return 0;
}

float4 main(float2 _31)
{
    gAccessCount = 0;
    int _41 = Z_i();
    float4 array[1] = { 0.0f.xxxx };
    array[_41] = _12_colorGreen * 0.5f;
    int _45 = Z_i();
    array[_45].w = 2.0f;
    int _50 = Z_i();
    array[_50].y *= 4.0f;
    int _56 = Z_i();
    float3 _66 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), array[_56].yzw);
    array[_56] = float4(array[_56].x, _66.x, _66.y, _66.z);
    int _69 = Z_i();
    float4 _76 = array[_69].zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    array[_69] = float4(_76.w, _76.y, _76.x, _76.z);
    int _79 = Z_i();
    int _83 = Z_i();
    float _90 = 0.0f;
    if (array[_83].w <= 1.0f)
    {
        int _94 = Z_i();
        _90 = array[_94].z;
    }
    else
    {
        int _98 = Z_i();
        _90 = float(_98);
    }
    array[_79].x += _90;
    bool _114 = false;
    if (gAccessCount == 8)
    {
        _114 = all(bool4(array[0].x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, array[0].y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, array[0].z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, array[0].w == float4(1.0f, 1.0f, 0.25f, 1.0f).w));
    }
    else
    {
        _114 = false;
    }
    float4 _115 = 0.0f.xxxx;
    if (_114)
    {
        _115 = _12_colorGreen;
    }
    else
    {
        _115 = _12_colorRed;
    }
    return _115;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    float4 _24 = main(_22);
    sk_FragColor = _24;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
