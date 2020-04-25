cbuffer LightCBuf
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    float padding[2];
};

Texture2D tex;
Texture2D spec;

SamplerState splr;

float4 main(float3 worldPos : Position, float3 n : Normal, float2 tc : Texcoord) : SV_Target
{
    //Fragment to light vector data
    const float3 vToL = lightPos - worldPos;
    const float distToL = length(vToL);
    const float3 dirToL = vToL / distToL;
    
    //Attenuation
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
    
    //Diffuse Intensity
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    
    //Reflected Light Vector
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
    
    //Calculate Specular intensity
    const float4 specularSample = spec.Sample(splr, tc);
    const float3 specularColorIntensity = specularSample.rgb;
    const float specularPower = specularSample.a;
    const float3 specular = att * specularColorIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    
    //Calculate final colour with texture and diffuse color
    return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);

}