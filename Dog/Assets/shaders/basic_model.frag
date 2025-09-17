#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in uint textureIndex;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D uTextures[];

void main()
{
	if (textureIndex == -1)
	{
		outColor = vec4(fragNormal, 1.0);
	}
	else
	{
		outColor = texture(uTextures[textureIndex], fragTexCoord);
		if (outColor.a < 0.1)
		{
			discard;
		}
	}
}