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
  uint boneOffset;
};

layout(std430, set = 1, binding = 1) buffer readonly InstanceDataBuffer {
  Instance instances[10000];
} instanceData;

struct VQS {
    vec4 rotation;    // Quat
    vec3 translation; // Vector
    vec3 scale;       // Scale
};

// Update your buffer to use the new struct
layout(set = 1, binding = 2) buffer readonly BoneBuffer {
	VQS finalBoneVQS[10000];
} animationData;

// Rotate vector by a quat
vec3 rotate(vec4 q, vec3 v) {
    vec3 t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

void main() 
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    
    uint offset = instanceData.instances[gl_InstanceIndex].boneOffset;
    bool validBoneFound = false;
    if (offset != 10001)
    {
        for (uint i = 0; i < 4 ; i++)
        {
            if(boneIds[i] == -1) continue;

            VQS transform = animationData.finalBoneVQS[offset + boneIds[i]];

            // --- Position Transformation ---
            vec3 localPosition = rotate(transform.rotation, position * transform.scale) + transform.translation;

            // --- Normal Transformation
            vec3 inverseScale = vec3(1.0) / transform.scale;
            vec3 localNormal = rotate(transform.rotation, normal * inverseScale);

            // --- Accumulate weighted results ---
            totalPosition += vec4(localPosition, 1.0f) * weights[i];
            totalNormal += localNormal * weights[i];

            validBoneFound = true;
        }

    }
    
    if (!validBoneFound || totalPosition == vec4(0.0))
    {
		totalPosition = vec4(position, 1.0);
        totalNormal = normal;
	}

    gl_Position = uniforms.projectionView * instanceData.instances[gl_InstanceIndex].model * vec4(totalPosition.xyz, 1.0);

    fragColor = color;
    fragNormal = normalize(totalNormal);
    fragTexCoord = texCoord;
    textureIndex = instanceData.instances[gl_InstanceIndex].textureIndex;
}
