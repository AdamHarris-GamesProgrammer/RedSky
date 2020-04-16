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

float4 main(float3 worldPos : Position, float3 n : Normal, float3 color : Color) : SV_Target
{
    //Light vector data
    const float3 vTol = lightPos - worldPos;
    const float distToL = length(vTol);
    const float3 dirToL = vTol / distToL;
    
    //attenuation
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
    
    //diffuse intensity
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    
    //Reflected light vectors
    const float3 w = n * dot(vTol, n);
    const float3 r = w * 2.0f - vTol;
    
    //claculate specular highlight colour
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    
    return float4(saturate((diffuse + ambient + specular) * color), 1.0f);
}