#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;

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

void main() 
{
    gl_Position = uniforms.projectionView * instanceData.instances[gl_InstanceIndex].model * vec4(position.xyz, 1.0);

    fragColor = color;
    fragNormal = normal;
    fragTexCoord = texCoord;
    textureIndex = instanceData.instances[gl_InstanceIndex].textureIndex;
}
