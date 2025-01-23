#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) flat out uint outInstanceIndex;

#include "camera_set.glsl"
#include "instance_set.glsl"

// -------------------------------------------------------------------
// Glyphs --------------------------------------------------------
layout(set = 2, binding = 0) buffer readonly GlyphDataBuffer {
    vec4 glyphs[NL_MAX_GLYPHS];
} glyphData;

// -------------------------------------------------------------------
// Bones! ------------------------------------------------------------
struct AnimationData {
    mat4 finalBonesMatrices[NL_MAX_BONES];
};

layout(set = 2, binding = 2) buffer readonly BoneBuffer {
	AnimationData bones[NL_MAX_MODELS];
} animationData;

// Global light data
const float cAmbient = 0.02;

void main() {
    // Fetch the instance-specific data
    int instanceIndex = gl_InstanceIndex;

    mat4 model = instanceData.instances[instanceIndex].modelMatrix;
    mat4 normalMat = instanceData.instances[instanceIndex].normalMatrix;
    uint animationIndex = instanceData.instances[instanceIndex].animationIndex;

    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);

    if (animationIndex == NL_INVALID_ANIMATION_INDEX) 
    {
        // No animations
		totalPosition = vec4(position, 1.0);
        totalNormal = normal;
    }
    else 
	{
        // Yes animations
        bool validBoneFound = false;

        for(int i = 0 ; i < NL_MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) continue;

            if(boneIds[i] >= NL_MAX_BONES) 
            {
                continue;
            }

            vec4 localPosition = animationData.bones[animationIndex].finalBonesMatrices[boneIds[i]] * vec4(position,1.0f);
            totalPosition += localPosition * weights[i];

            vec3 localNormal = mat3(animationData.bones[animationIndex].finalBonesMatrices[boneIds[i]]) * normal;
            totalNormal += localNormal * weights[i];

            validBoneFound = true;
        }

        if (!validBoneFound || totalPosition == vec4(0.0)) {
		    totalPosition = vec4(position, 1.0);
	    }
	}

    // Transform the vertex position to clip space
    //gl_Position = uniforms.projectionView * model * vec4(position, 1.0);
    if (instanceData.instances[instanceIndex].followCamera == 1) 
    {
        gl_Position = uniforms.projection * uniforms.view * model * vec4(totalPosition.xyz, 1.0);
    }
    else
    {    
        gl_Position = uniforms.projection * model * vec4(totalPosition.xyz, 1.0);
    }

    // Calculate the normal in world space
    vec3 normalWorldSpace = normalize(mat3(normalMat) * totalNormal);

    // Compute light intensity
    float lightIntensity = cAmbient + max(dot(normalWorldSpace, uniforms.directionToLight), 0.0);

    // Pass the color and texture coordinates to the fragment shader
    fragColor = lightIntensity * color;

    if (instanceData.instances[instanceIndex].charIndex != NL_INVALID_FONT_INDEX) 
    {
        uint charIndex = instanceData.instances[instanceIndex].charIndex - 32;
    
        vec4 texInfo = glyphData.glyphs[charIndex];
        
        float epsilon = 0.0005;
        float l = texInfo.x + epsilon; 
        float b = texInfo.w + epsilon; // flipped w/ t
        float r = texInfo.z - epsilon; 
        float t = texInfo.y - epsilon; // flipped w/ b
    
        if      (texCoord.x == 0 && texCoord.y == 0) fragTexCoord = vec2(l, t);
        else if (texCoord.x == 1 && texCoord.y == 0) fragTexCoord = vec2(r, t);
        else if (texCoord.x == 1 && texCoord.y == 1) fragTexCoord = vec2(r, b);
        else if (texCoord.x == 0 && texCoord.y == 1) fragTexCoord = vec2(l, b);
    }
    else {
        fragTexCoord = texCoord;
    }

    fragNormal = normalWorldSpace;
    outInstanceIndex = instanceIndex;
}
