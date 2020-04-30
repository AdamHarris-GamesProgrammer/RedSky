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
    bool normalMapEnabled;
    bool hasGloss;
    float specularPowerConst;
    float padding[1];
};

Texture2D tex;
Texture2D spec;
Texture2D nmap;

SamplerState splr;

float4 main(float3 viewPos : Position, float3 n : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
    if (normalMapEnabled)
    {
        const float3x3 tanToView = float3x3(
            normalize(tan),
            normalize(bitan),
            normalize(n)
        );
        
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
        n = normalSample * 2.0f - 1.0f;
        
        n = mul(n, tanToView);

    }
    
    //Fragment to light vector data
    const float3 vToL = lightPos - viewPos;
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
    const float3 specularReflectionColor = specularSample.rgb;
    float specularPower;
    if (hasGloss)
    {
        specularPower = pow(2.0f, specularSample.a * 13.0f);
    }
    else
    {
        specularPower = specularPowerConst;
    }
    
    
    const float3 specular = att * (diffuseColor * diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
    
  
    //Calculate final colour with texture and diffuse color
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);

}