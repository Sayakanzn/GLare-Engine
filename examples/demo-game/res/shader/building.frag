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

// Time for animations
uniform float time;

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

// Blueprint parameters
uniform float blueprintProgress = 0.0;                      // 0.0 = fully blueprint, 1.0 = fully normal
uniform vec3 blueprintColor = vec3(0.2, 0.6, 1.0);          // Blueprint blue color
uniform float blueprintAlpha = 0.67;                        // Base transparency
uniform vec2 blueprintFillDirection = vec2(0.0, -1.0);      // Fill direction
uniform float blueprintFillSoftness = 0.01;                 // Softness of the fill transition
uniform vec3 worldBoundsMin = vec3(-4.7, -3.14142, -4.7);   // World space bounds min
uniform vec3 worldBoundsMax = vec3(-3.18881, -0.5, -2.95);  // World space bounds max
uniform bool useWorldSpace = true;                          // Use world space for fill calculation

uniform float blueprintGridScale1 = 0.219; 
uniform float blueprintGridScale2 = 4.228; 
uniform float blueprintGridScale3 = 27.824;
uniform float blueprintLineIntensity = 1.5;
uniform float blueprintScanSpeed = 0.25;   
uniform float blueprintPulseSpeed = 2.0;   
uniform float blueprintHologramNoise = 0.0;

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

const float PI = 3.14159265359;

float blueprintGrid(vec2 uv, float scale) {
    vec2 grid = fract(uv * scale);
    vec2 dGrid = fwidth(uv * scale);
    
    float lineX = smoothstep(0.0, dGrid.x * 2.0, grid.x) * (1.0 - smoothstep(1.0 - dGrid.x * 2.0, 1.0, grid.x));
    float lineY = smoothstep(0.0, dGrid.y * 2.0, grid.y) * (1.0 - smoothstep(1.0 - dGrid.y * 2.0, 1.0, grid.y));
    
    return 1.0 - min(lineX, lineY);
}

float hexGrid(vec2 uv, float scale) {
    uv *= scale;
    vec2 r = vec2(1.0, 1.732);
    vec2 h = r * 0.5;
    vec2 a = mod(uv, r) - h;
    vec2 b = mod(uv - h, r) - h;
    float dist = min(dot(a, a), dot(b, b));
    return 1.0 - smoothstep(0.0, 0.05, dist);
}

// Simple noise function for holographic effect
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

vec3 calculateBlueprintPattern(vec2 uv, vec3 worldPos, vec3 normal, float animTime) {
    float grid1 = blueprintGrid(uv, blueprintGridScale1) * 0.3;
    float grid2 = blueprintGrid(uv, blueprintGridScale2) * 0.5;
    float grid3 = blueprintGrid(uv, blueprintGridScale3) * 0.2;
    
    float hexPattern = hexGrid(uv + vec2(animTime * 0.1), blueprintGridScale1 * 0.7) * 0.2;
    
    float basePattern = max(max(grid1, grid2), max(grid3, hexPattern));
    
    float scanLine = sin(worldPos.y * 10.0 - animTime * blueprintScanSpeed * 10.0) * 0.5 + 0.5;
    scanLine = pow(scanLine, 3.0) * 0.3;
    
    float pulse = sin(animTime * blueprintPulseSpeed) * 0.5 + 0.5;
    float glowPattern = pulse * 0.2;
    
    float edgeHighlight = 1.0 - abs(dot(normalize(viewPosition - worldPos), normal));
    edgeHighlight = pow(edgeHighlight, 2.0) * 0.5;
    
    float holoNoise = noise(uv * 100.0 + vec2(animTime * 2.0)) * blueprintHologramNoise;
    
    float dataFlow = sin(uv.x * 50.0 + animTime * 3.0) * sin(uv.y * 30.0 - animTime * 2.0);
    dataFlow = smoothstep(0.8, 1.0, dataFlow) * 0.15;
    
    float totalPattern = basePattern * blueprintLineIntensity + scanLine + glowPattern + edgeHighlight + holoNoise + dataFlow;
    
    vec3 color = blueprintColor;
    
    color = mix(color * 0.5, color * 1.5, totalPattern);
    
    vec3 highlightColor = vec3(0.0, 1.0, 1.0);
    color = mix(color, highlightColor, dataFlow + scanLine * 0.3);
    
    color = max(color, blueprintColor * 0.2);
    
    return color;
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
    float animTime = time;
    
    vec3 positionForFill;
    vec3 boundsMin, boundsMax;
    
    if (useWorldSpace) {
        positionForFill = FragPos;
        boundsMin = worldBoundsMin;
        boundsMax = worldBoundsMax;
    }
    else {
        positionForFill = (inverse(model) * vec4(FragPos, 1.0)).xyz;
        boundsMin = worldBoundsMin;
        boundsMax = worldBoundsMax;
    }
    
    vec3 boundsSize = boundsMax - boundsMin;
    vec3 normalizedPos = (positionForFill - boundsMin) / boundsSize;
    normalizedPos = clamp(normalizedPos, 0.0, 1.0);
    
    float fillPosition;
    if (abs(blueprintFillDirection.x) > abs(blueprintFillDirection.y)) {
        fillPosition = blueprintFillDirection.x > 0.0 ? normalizedPos.x : (1.0 - normalizedPos.x);
    } 
    else {
        fillPosition = blueprintFillDirection.y > 0.0 ? normalizedPos.y : (1.0 - normalizedPos.y);
    }
    
    float isBlueprint = step(fillPosition, 1.0 - blueprintProgress);
    
    // Sample textures
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
    
    if (debugMode == 1) {
        FragColor = vec4(baseColor, texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    } 
    else if (debugMode == 2) {
        FragColor = vec4(norm * 0.5 + 0.5, texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    } 
    else if (debugMode == 3) {
        FragColor = vec4(vec3(roughness), texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    } 
    else if (debugMode == 4) {
        FragColor = vec4(vec3(metallic), texColor.a);
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    } 
    else if (debugMode == 5) {
        vec3 flatWhite = vec3(1.0);
        vec3 lightOnlyResult = ambientStrength * 2.0 * flatWhite;
        
        float totalShadow = 0.0;
        int shadowCastingLights = 0;
        
        for(int i = 0; i < directionalLightCount; i++) {
            DirectionalLight light = directionalLights[i];
            vec3 lightDir = normalize(-light.direction);
            
            float shadow = 0.0;
            if(light.castShadows) {
                shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]) * 0.7;
                totalShadow += shadow;
                shadowCastingLights++;
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
                totalShadow += shadow;
                shadowCastingLights++;
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
        return;
    }
    else if (debugMode == 6) {
        float totalShadow = 0.0;
        int shadowCastingLights = 0;
        
        for(int i = 0; i < directionalLightCount; i++) {
            DirectionalLight light = directionalLights[i];
            if(light.castShadows) {
                float shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]);
                totalShadow += shadow;
                shadowCastingLights++;
            }
        }
        
        for(int i = 0; i < pointLightCount; i++) {
            PointLight light = pointLights[i];
            if(light.castShadows && light.shadowMapIndex >= 0) {
                float shadow = calculatePointShadow(FragPos, light, pointLightShadowCubemaps[light.shadowMapIndex]);
                totalShadow += shadow;
                shadowCastingLights++;
            }
        }
        
        if (shadowCastingLights > 0) {
            float avgShadow = totalShadow / float(shadowCastingLights);
            FragColor = vec4(vec3(avgShadow), texColor.a);
        } 
        else {
            FragColor = vec4(0.0, 0.0, 0.0, texColor.a);
        }
        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    
    vec3 finalColor;
    float finalAlpha;
    
    if (isBlueprint > 0.5) {
        vec3 blueprintPattern = calculateBlueprintPattern(TexCoord, FragPos, norm, animTime);
        
        float depthFade = length(viewPosition - FragPos) * 0.01;
        depthFade = clamp(1.0 - depthFade, 0.3, 1.0);
        
        finalColor = blueprintPattern * depthFade;
        
        float patternIntensity = dot(blueprintPattern, vec3(0.333));
        finalAlpha = mix(blueprintAlpha * 0.5, min(1.0, blueprintAlpha * 2.0), patternIntensity);
        
        BloomColor = vec4(blueprintPattern * 0.3, finalAlpha);
    } 
    else {
        vec3 ambientColor = mix(baseColor, baseColor * 0.3, metallic);
        vec3 result = ambientStrength * 2.0 * ambientColor;
        
        for(int i = 0; i < directionalLightCount; i++) {
            DirectionalLight light = directionalLights[i];
            vec3 lightDir = normalize(-light.direction);
            
            float shadow = 0.0;
            if(light.castShadows) {
                shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]) * 0.7;
            }
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, 
                                                   light.intensity, baseColor, metallic, roughness);
            result += radiance * (1.0 - shadow);
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
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, 
                                                   light.intensity * attenuation, baseColor, metallic, roughness);
            result += radiance * (1.0 - shadow);
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
            
            vec3 radiance = calculateCartoonLighting(norm, viewDir, lightDir, light.color, light.intensity * attenuation * intensity, baseColor, metallic, roughness);
            result += radiance;
        }
        
        result += emissive;
        finalColor = result;
        finalAlpha = texColor.a;
        
        vec3 bloomContribution = emissive * emissiveBloomBoost;
        float brightness = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > bloomThreshold) {
            bloomContribution += finalColor * (brightness - bloomThreshold);
        }
        BloomColor = vec4(bloomContribution, finalAlpha);
    }
    
    float edgeDistance = abs(fillPosition - (1.0 - blueprintProgress));
    if (edgeDistance < blueprintFillSoftness * 3.0) {
        float edgeFactor = edgeDistance / (blueprintFillSoftness * 3.0);
        
        float edgePulse = sin(animTime * 4.0) * 0.5 + 0.5;
        vec3 edgeGlow = mix(blueprintColor * 3.0, vec3(0.0, 1.0, 1.0) * 2.0, edgePulse);
        
        finalColor = mix(edgeGlow, finalColor, edgeFactor);
        finalAlpha = mix(1.0, finalAlpha, edgeFactor);
        
        BloomColor = vec4(edgeGlow * (1.0 - edgeFactor) * 0.5, finalAlpha);
    }
    
    // Output final color
    FragColor = vec4(finalColor, finalAlpha);
}