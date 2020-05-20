#include "ShaderOperations.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

cbuffer ObjectCBuf
{
    float3 materialColor;
    float3 specularColor;
    float specularWeight;
    float specularGloss;
};

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal) : SV_Target
{
    //renomalise normals
    viewNormal = normalize(viewNormal);

    //Light Vector
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
    
    //attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
    
    //diffuse
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
    
    //Specular
    const float3 specular = Speculate(
        diffuseColor * diffuseIntensity * specularColor, specularWeight, viewNormal,
        lv.vToL, viewFragPos, att, specularGloss
    );
    
    return float4(saturate((diffuse + ambient) * materialColor + specular), 1.0f);
}