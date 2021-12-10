cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_a : packoffset(c0);
    float _10_b : packoffset(c0.y);
    float _10_c : packoffset(c0.z);
    float4 _10_d : packoffset(c1);
    float4 _10_e : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvRefract(float i, float n, float eta)
{
    float NoI = n * i;
    float NoI2 = NoI * NoI;
    float k = 1.0 - eta * eta * (1.0 - NoI2);
    if (k < 0.0)
    {
        return 0.0;
    }
    else
    {
        return eta * i - (eta * NoI + sqrt(k)) * n;
    }
}

void frag_main()
{
    sk_FragColor.x = spvRefract(_10_a, _10_b, _10_c);
    sk_FragColor = refract(_10_d, _10_e, _10_c);
    sk_FragColor = float4(float2(0.5f, -0.866025388240814208984375f).x, float2(0.5f, -0.866025388240814208984375f).y, sk_FragColor.z, sk_FragColor.w);
    sk_FragColor = float4(float3(0.5f, 0.0f, -0.866025388240814208984375f).x, float3(0.5f, 0.0f, -0.866025388240814208984375f).y, float3(0.5f, 0.0f, -0.866025388240814208984375f).z, sk_FragColor.w);
    sk_FragColor = float4(0.5f, 0.0f, 0.0f, -0.866025388240814208984375f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
