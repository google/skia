static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    bool3 B = bool3(false, false, false);
    B.x = true;
    B.y = true;
    B.z = true;
    float3 F = 0.0f.xxx;
    F.x = 1.230000019073486328125f;
    F.y = 0.0f;
    F.z = 1.0f;
    int3 I = int3(0, 0, 0);
    I.x = 1;
    I.y = 1;
    I.z = 1;
    bool _67 = false;
    if (B.x)
    {
        _67 = B.y;
    }
    else
    {
        _67 = false;
    }
    bool _72 = false;
    if (_67)
    {
        _72 = B.z;
    }
    else
    {
        _72 = false;
    }
    return float4((F.x * F.y) * F.z, float(_72), 0.0f, float((I.x * I.y) * I.z));
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
