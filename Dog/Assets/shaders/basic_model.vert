#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) flat out uint textureIndex;

layout(set = 0, binding = 0) uniform Uniforms {
    mat4 projectionView;
    mat4 projection;
    mat4 view;
} uniforms;

struct Instance {
  mat4 model;
  uint textureIndex;
};

layout(std430, set = 1, binding = 1) buffer readonly InstanceDataBuffer {
  Instance instances[DOG_MAX_INSTANCES];
} instanceData;

layout(set = 1, binding = 2) buffer readonly BoneBuffer {
	mat4 finalBonesMatrices[DOG_MAX_BONES];
} animationData;

void main() 
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    
    {
        // Yes animations
        bool validBoneFound = false;

        for(int i = 0 ; i < DOG_MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) continue;

            if(boneIds[i] >= DOG_MAX_BONES) 
            {
                continue;
            }

            vec4 localPosition = animationData.finalBonesMatrices[boneIds[i]] * vec4(position,1.0f);
            totalPosition += localPosition * weights[i];

            vec3 localNormal = mat3(animationData.finalBonesMatrices[boneIds[i]]) * normal;
            totalNormal += localNormal * weights[i];

            validBoneFound = true;
        }

        if (!validBoneFound || totalPosition == vec4(0.0)) {
		    totalPosition = vec4(position, 1.0);
	    }
    }

    gl_Position = uniforms.projectionView * instanceData.instances[gl_InstanceIndex].model * vec4(totalPosition.xyz, 1.0);

    fragColor = color;
    fragNormal = normal;
    fragTexCoord = texCoord;
    textureIndex = instanceData.instances[gl_InstanceIndex].textureIndex;
}
