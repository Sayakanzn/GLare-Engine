#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec4 aJointIndices;
layout (location = 6) in vec4 aJointWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int isSkinned;
uniform mat4 jointMatrices[100];

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

void main() {
    vec4 finalPosition;
    vec3 finalNormal = aNormal;
    vec3 finalTangent = aTangent;
    vec3 finalBitangent = aBitangent;
    
    if (isSkinned == 1) {
        finalPosition = vec4(0.0);
        finalNormal = vec3(0.0);
        finalTangent = vec3(0.0);
        finalBitangent = vec3(0.0);
        
        for (int i = 0; i < 4; i++) {
            float weight = aJointWeights[i];
            if (weight > 0.0) {
                int jointIndex = int(aJointIndices[i]);
                mat4 jointMatrix = jointMatrices[jointIndex];
                
                finalPosition += weight * (jointMatrix * vec4(aPos, 1.0));
                
                mat3 jointRotation = mat3(jointMatrix);
                finalNormal += weight * (jointRotation * aNormal);
                finalTangent += weight * (jointRotation * aTangent);
                finalBitangent += weight * (jointRotation * aBitangent);
            }
        }
    }
    else {
        finalPosition = vec4(aPos, 1.0);
    }
    
    vec4 worldPosition = model * finalPosition;
    
    FragPos = worldPosition.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * finalNormal);
    vec3 T = normalize(normalMatrix * finalTangent);
    vec3 B = normalize(normalMatrix * finalBitangent);
    
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);
    
    TBN = mat3(T, B, N);
    
    Normal = N;
    
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * worldPosition;
}