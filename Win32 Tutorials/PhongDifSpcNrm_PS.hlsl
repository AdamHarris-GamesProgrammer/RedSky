#include "ShaderOperations.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl"

cbuffer ObjectCBuf
{
    bool useGlossAlpha;
    float3 specularColor;
    float specularWeight;
    float specularGloss;
    bool useNormalMap;
    float normalMapWeight;
};

Texture2D tex;
Texture2D spec;
Texture2D nmap;

SamplerState splr;


float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
      
    float4 dtex = tex.Sample(splr, tc);
    
#ifdef MASKED_OBJECT
    clip(dtex.a < 0.1f ? -1 : 1);
    
    if (dot(viewNormal, viewFragPos) >= 0.0f)
    {
        viewNormal = -viewNormal;
    }
#endif    

    viewNormal = normalize(viewNormal);
    
    if (useNormalMap)
    {
        viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, splr);
    }
    
    //Fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
    
    float3 specularReflectionColor;
    float specularPower = specularGloss;

    const float4 specularSample = spec.Sample(splr, tc);
    specularReflectionColor = specularSample.rgb;
    if (useGlossAlpha)
    {
        specularPower = pow(2.0f, specularSample.a * 13.0f);
    }
    
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
    
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
    
    const float3 specularReflected = Speculate(
        diffuseColor * diffuseIntensity * specularReflectionColor, specularWeight, viewNormal,
        lv.vToL, viewFragPos, att, specularPower
    );

    
    //Calculate final colour with texture and diffuse color
    return float4(saturate((diffuse + ambient) * dtex.rgb + specularReflected), 1.0f);

}