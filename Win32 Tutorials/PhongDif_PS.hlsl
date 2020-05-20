#include "ShaderOperations.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

cbuffer ObjectCBuf
{
    float3 specularColor;
    float specularWeight;
    float specularGloss;
};

Texture2D tex;

SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float2 tc : Texcoord) : SV_Target
{
    //renomalise normals
    viewNormal = normalize(viewNormal);
    
    //Fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
    
    
    //Attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
    //Diffuse Intensity
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
    
    //Calculate Specular intensity
    const float3 specular = Speculate(
        diffuseColor * diffuseIntensity * specularColor, specularWeight, 
        viewNormal, lv.vToL, viewFragPos, att, specularGloss
    );
    
    //Calculate final colour with texture and diffuse color
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}