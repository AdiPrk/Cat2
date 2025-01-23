#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D uTextures[];  // Texture sampler

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  int textureIndex;
} push;

void main() {
  vec3 surfaceNormal = normalize(fragNormalWorld);

  // Fetch texture color
  vec4 texColor = vec4(1.0);
  if (push.textureIndex != 9999) {
    texColor = texture(uTextures[push.textureIndex], fragTexCoord);
  }

  if (push.textureIndex == 9999) {
    outColor = vec4(surfaceNormal, 1.f);
  }
  else {
    outColor = vec4(texColor);
  }
}
