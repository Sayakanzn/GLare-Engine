#version 410 core

layout (location = 0) out vec4 FragColor; 
layout (location = 1) out vec4 BloomColor;

// Input from vertex shader
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

// Model matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPosition;

// Material textures
uniform sampler2D baseColorTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D emissiveTexture;

// Texture boolean controls
uniform bool baseColorTextureBool = false;
uniform bool normalTextureBool = false;
uniform bool metallicRoughnessTextureBool = false;
uniform bool emissiveTextureBool = false;

// PBR parameters
uniform vec4 baseColorFactor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float metallicFactor = 1.0;
uniform float roughnessFactor = 1.0;
uniform float normalScale = 1.0;
uniform vec3 emissiveFactor = vec3(0.0, 0.0, 0.0);
uniform float baseMetallic = 0.0;
uniform float baseRoughness = 0.5;
uniform float normalMapStrength = 1.0;

// Cartoon lighting parameters
uniform float cartoonDiffuseStrength = 0.8;
uniform float cartoonSpecularStrength = 0.2;
uniform float cartoonSpecularSize = 32.0;
uniform float cartoonMinLighting = 0.3;
uniform float cartoonNormalFlatten = 0.5;

// Melting snow parameters
uniform float width = 0.0;
uniform float meltSoftness = 0.3;
uniform float meltNoiseScale = 10.0;
uniform float meltNoiseStrength = 0.02;
uniform vec2 meltCenter = vec2(0.0, 0.0);

// Bloom controls
uniform float bloomThreshold = 1.0;
uniform float emissiveBloomBoost = 1.0;
uniform float baseColorBloomFactor = 2.0;

// Lighting structures
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool castShadows;
    mat4 lightSpaceMatrix;
    float shadowBias;
};
struct PointLight {
    vec3 position;
    vec3 color;
    float farPlane;
    float intensity;
    float radius;
    float constant;
    float linear;
    float quadratic;
    bool castShadows;
    int shadowMapIndex;
    float shadowBias;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float radius;
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
};

// Lighting uniforms
uniform int directionalLightCount;
uniform int pointLightCount;
uniform int spotLightCount;
uniform DirectionalLight directionalLights[2];
uniform PointLight pointLights[8];
uniform SpotLight spotLights[8];
uniform sampler2D directionalLightShadowMaps[2];
uniform samplerCube pointLightShadowCubemaps[4];
uniform float ambientStrength = 0.1;

uniform int debugMode = 0;

// Constants
const float PI = 3.14159265359;

// Simple noise function for organic melting pattern
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

vec3 calculateCartoonLighting(vec3 N, vec3 V, vec3 L, vec3 lightColor, float lightIntensity, vec3 baseColor, float metallic, float roughness) {
    float NdotL = dot(N, L);
    
    float diffuse = max(NdotL, 0.0);
    
    diffuse = mix(cartoonMinLighting, 1.0, diffuse);
    diffuse = pow(diffuse, 0.7);
    
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), mix(cartoonSpecularSize, cartoonSpecularSize * 0.5, roughness));
    
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    vec3 specularColor = mix(vec3(1.0), F0, metallic);
    
    vec3 diffuseColor = mix(baseColor, vec3(0.0), metallic) * diffuse * cartoonDiffuseStrength;
    vec3 finalSpecular = specularColor * spec * mix(cartoonSpecularStrength, cartoonSpecularStrength * 2.0, metallic);
    
    return (diffuseColor + finalSpecular) * lightColor * lightIntensity;
}

float calculateShadow(vec3 fragPos, DirectionalLight light, sampler2D shadowMap) {
    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 || 
       projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = light.shadowBias;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    const int pcfSize = 2;
    
    for(int x = -pcfSize; x <= pcfSize; ++x) {
        for(int y = -pcfSize; y <= pcfSize; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    shadow /= ((pcfSize * 2 + 1) * (pcfSize * 2 + 1));
    
    return shadow;
}

float calculatePointShadow(vec3 fragPos, PointLight light, samplerCube shadowCubemap) {
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    
    if(currentDepth > light.farPlane) {
        return 0.0;
    }
    
    vec3 sampleDirection = normalize(fragToLight);
    float closestDepth = texture(shadowCubemap, sampleDirection).r;
    closestDepth *= light.farPlane;
    
    float bias = light.shadowBias;
    float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

void main() {
    vec3 objectSpacePos = (inverse(model) * vec4(FragPos, 1.0)).xyz;
    
    vec2 xzPos = vec2(objectSpacePos.x - meltCenter.x, objectSpacePos.z - meltCenter.y);
    float distFromCenter = length(xzPos);
    
    float noiseValue = noise(xzPos * meltNoiseScale);
    noiseValue += noise(xzPos * meltNoiseScale * 2.0) * 0.5;
    noiseValue += noise(xzPos * meltNoiseScale * 4.0) * 0.25;
    noiseValue *= meltNoiseStrength;
    
    float meltRadius = width;
    float meltThreshold = meltRadius + (noiseValue - 0.5 * meltNoiseStrength);
    
    float meltFactor = smoothstep(meltThreshold - meltSoftness, meltThreshold + meltSoftness, distFromCenter);
    
    float baseAlpha = 1.0;
    if (width > 0.0) {
        baseAlpha = meltFactor;
        
        if (meltFactor > 0.1 && meltFactor < 0.9) {
            float edgeNoise = noise(xzPos * meltNoiseScale * 8.0) * 0.1;
            baseAlpha = clamp(baseAlpha + edgeNoise, 0.0, 1.0);
        }
    }
    
    if (baseAlpha < 0.01) {
        discard;
    }
    
    vec4 texColor;
    if (baseColorTextureBool) {
        texColor = texture(baseColorTexture, TexCoord) * baseColorFactor;
    } 
    else {
        texColor = baseColorFactor;
    }

    float metallic = baseMetallic;
    float roughness = baseRoughness;
    
    if (metallicRoughnessTextureBool) {
        vec3 metallicRoughnessValue = texture(metallicRoughnessTexture, TexCoord).rgb;
        metallic = metallicRoughnessValue.b * metallicFactor;
        roughness = metallicRoughnessValue.g * roughnessFactor;
    } 
    else {
        metallic = baseMetallic * metallicFactor;
        roughness = baseRoughness * roughnessFactor;
    }
    
    texColor.a *= baseAlpha;
    
    if (texColor.a < 0.01) {
        discard;
    }
    
    vec3 normalMapValue;
    if (normalTextureBool) {
        normalMapValue = texture(normalTexture, TexCoord).rgb;
        normalMapValue = normalMapValue * 2.0 - 1.0;
        normalMapValue = normalize(mix(vec3(0.0, 0.0, 1.0), normalMapValue, cartoonNormalFlatten));
    } 
    else {
        normalMapValue = vec3(0.0, 0.0, 1.0);
    }
    
    float finalNormalStrength = normalMapStrength * normalScale * cartoonNormalFlatten;
    vec3 baseNormal = vec3(0.0, 0.0, 1.0);
    normalMapValue = normalize(mix(baseNormal, normalMapValue, finalNormalStrength));
    
    vec3 norm = normalize(TBN * normalMapValue);
    
    vec3 emissive;
    if (emissiveTextureBool) {
        emissive = texture(emissiveTexture, TexCoord).rgb * emissiveFactor;
    } 
    else {
        emissive = emissiveFactor * baseColorBloomFactor;
    }
    
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 baseColor = texColor.rgb;
    
    vec3 ambientColor = mix(baseColor, baseColor * 0.3, metallic);
    vec3 result = ambientStrength * 2.0 * ambientColor;
    
    float totalShadow = 0.0;
    int shadowCastingLights = 0;
    
    // Process directional lights
    for(int i = 0; i < directionalLightCount; i++) {
        DirectionalLight light = directionalLights[i];
        vec3 lightDir = normalize(-light.direction);
        
        float shadow = 0.0;
        if(light.castShadows) {
            shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]);
            shadow = shadow * 0.7;
            totalShadow += shadow;
            shadowCastingLights++;
        }
        
        vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity, baseColor, metallic, roughness);
        result += radiance * (1.0 - shadow);
    }

    // Process point lights
    for(int i = 0; i < pointLightCount; i++) {
        PointLight light = pointLights[i];
        vec3 lightDir = normalize(light.position - FragPos);
        
        float distance = length(light.position - FragPos);
        if(distance > light.radius) continue;
        
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
        
        float shadow = 0.0;
        if(light.castShadows && light.shadowMapIndex >= 0) {
            shadow = calculatePointShadow(FragPos, light, pointLightShadowCubemaps[light.shadowMapIndex]);
            shadow = shadow * 0.7;
            totalShadow += shadow;
            shadowCastingLights++;
        }
        
        vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity * attenuation, baseColor, metallic, roughness);
        result += radiance * (1.0 - shadow);
    }
    
    // Process spot lights
    for(int i = 0; i < spotLightCount; i++) {
        SpotLight light = spotLights[i];
        vec3 lightDir = normalize(light.position - FragPos);
        
        float distance = length(light.position - FragPos);
        if(distance > light.radius) continue;
        
        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
        
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.innerCutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        
        vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity * attenuation * intensity, baseColor, metallic, roughness);
        result += radiance;
    }
    
    result += emissive;
    
    if (debugMode == 1) {
        FragColor = vec4(baseColor, texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else if (debugMode == 2) {
        FragColor = vec4(norm * 0.5 + 0.5, texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else if (debugMode == 3) {
        FragColor = vec4(vec3(roughness), texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else if (debugMode == 4) {
        FragColor = vec4(vec3(metallic), texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else if (debugMode == 5) {
        vec3 flatWhite = vec3(1.0);
        vec3 lightOnlyResult = ambientStrength * 2.0 * flatWhite;
        
        for(int i = 0; i < directionalLightCount; i++) {
            DirectionalLight light = directionalLights[i];
            vec3 lightDir = normalize(-light.direction);
            
            float shadow = 0.0;
            if(light.castShadows) {
                shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]) * 0.7;
            }
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity, flatWhite, 0.0, 0.5);
            lightOnlyResult += radiance * (1.0 - shadow);
        }

        for(int i = 0; i < pointLightCount; i++) {
            PointLight light = pointLights[i];
            vec3 lightDir = normalize(light.position - FragPos);
            
            float distance = length(light.position - FragPos);
            if(distance > light.radius) continue;
            
            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
            
            float shadow = 0.0;
            if(light.castShadows && light.shadowMapIndex >= 0) {
                shadow = calculatePointShadow(FragPos, light, pointLightShadowCubemaps[light.shadowMapIndex]) * 0.7;
            }
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity * attenuation, flatWhite, 0.0, 0.5);
            lightOnlyResult += radiance * (1.0 - shadow);
        }
        
        for(int i = 0; i < spotLightCount; i++) {
            SpotLight light = spotLights[i];
            vec3 lightDir = normalize(light.position - FragPos);
            
            float distance = length(light.position - FragPos);
            if(distance > light.radius) continue;
            
            float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
            
            float theta = dot(lightDir, normalize(-light.direction));
            float epsilon = light.innerCutoff - light.outerCutoff;
            float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity * attenuation * intensity, flatWhite, 0.0, 0.5);
            lightOnlyResult += radiance;
        }
        
        FragColor = vec4(lightOnlyResult, texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if (debugMode == 6) {
        if (shadowCastingLights > 0) {
            float avgShadow = totalShadow / float(shadowCastingLights);
            FragColor = vec4(vec3(avgShadow), texColor.a);
        } 
        else {
            FragColor = vec4(0.0, 0.0, 0.0, texColor.a);
        }
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        FragColor = vec4(result, texColor.a);
        
        vec3 bloomContribution = vec3(0.0);
        
        bloomContribution += emissive * emissiveBloomBoost;
        
        float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > bloomThreshold) {
            bloomContribution += result * (brightness - bloomThreshold);
        }
        
        BloomColor = vec4(bloomContribution, texColor.a);
    }
}