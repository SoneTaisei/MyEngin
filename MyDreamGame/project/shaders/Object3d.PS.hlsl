#include "object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b3);

struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

float4 main(VertexShaderOutput input) : SV_TARGET {
    float4 outputColor;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a < 0.5f) {
        discard;
    }
    
    if (gMaterial.lightingType == 2) {
        float3 normal = normalize(input.normal);
        float3 lightDir = normalize(-gDirectionalLight.direction);
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        
        float rim = 1.0f - saturate(dot(normal, toEye));
        float fresnel = pow(rim, 3.0f);
        
        float3 halfVector = normalize(lightDir + toEye);
        float specularPow = pow(saturate(dot(normal, halfVector)), 256.0f);
        float3 specular = float3(1.0f, 1.0f, 1.0f) * gDirectionalLight.intensity * specularPow;
        outputColor.rgb = textureColor.rgb * gMaterial.color.rgb + specular + (fresnel * 0.5f);
        
        float alphaBase = 0.3f;
        outputColor.a = saturate(alphaBase + fresnel) * gMaterial.color.a;
    
    } else if (gMaterial.lightingType != 0) {
        
        float3 normal = normalize(input.normal);
        float3 lightDir = normalize(-gDirectionalLight.direction);
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        
        float NdotL = dot(normal, lightDir);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f); // Half-Lambert
        if (gMaterial.lightingType == 1) {
            cos = saturate(NdotL); // Lambert
        }
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        float3 halfVector = normalize(lightDir + toEye);
        
        float NDotH = dot(normal, halfVector);
        
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        outputColor.rgb = diffuse + specular;
        outputColor.a = gMaterial.color.a * textureColor.a;

    } else {
        outputColor = gMaterial.color * textureColor;
    }

    return outputColor;
}