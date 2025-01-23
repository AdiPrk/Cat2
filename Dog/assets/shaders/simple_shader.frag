#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragNormal;
layout(location = 3) flat in uint instanceIndex;

layout (location = 0) out vec4 outColor;

#include "instance_set.glsl"

// -------------------------------------------------------------------
// Textures ----------------------------------------------------------
layout(set = 2, binding = 1) uniform sampler2D uTextures[NL_MAX_TEXTURES];

// -------------------------------------------------------------------
// Calculates the screen-space pixel range of the glyph
float screenPxRange() 
{
    uint fragTextureIndex = instanceData.instances[instanceIndex].textureIndex;

    const float pxRange = 2.0;

    vec2 unitRange = vec2(pxRange)/vec2(textureSize(uTextures[fragTextureIndex], 0));

    vec2 screenTexSize = vec2(1.0)/fwidth(fragTexCoord);

    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

// Calculates the median of three values
float median(float r, float g, float b) 
{
    return max(min(r, g), min(max(r, g), b));
}

// Constants
const float ALPHA_THRESHOLD = 0.5; // Sharp cutoff for alpha testing
const vec4 OUTLINE_COLOR = vec4(0.0, 0.0, 1.0, 1.0); // Blue outline
const float OUTLINE_WIDTH = 0.1; // Width of the outline

void main()
{
    uint fragTextureIndex = instanceData.instances[instanceIndex].textureIndex;

    if (fragTextureIndex == NL_NO_TEXTURES) 
    {
        // No textures, use vertex colors instead!
        outColor = vec4(fragColor, 1.0);
    }
    else if (instanceData.instances[instanceIndex].charIndex == NL_INVALID_FONT_INDEX)
    {
        // Directly use the texture color
		outColor = texture(uTextures[fragTextureIndex], fragTexCoord);
    }
    else 
    {
        // It's a glyph!
        vec3 msd = texture(uTextures[fragTextureIndex], fragTexCoord).rgb;

        // Calculate signed distance
        float sd = median(msd.r, msd.g, msd.b);

        // Calculate screen space distance. 
        // Center at 0, so values less than zero are inside of the glyph
        float screenPxDistance = screenPxRange()*(sd - 0.5);

        // Set the color based on the distance
        float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

        vec4 textColor = vec4(1.0, 0.0, 0.0, 1.0);
        vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.0); 

        vec3 newColor = bgColor.rgb * (1.0 - opacity) + textColor.rgb * opacity;
        float newAlpha = bgColor.a * (1.0 - opacity) + textColor.a * opacity;

        if (newAlpha < 0.5) {
            outColor = vec4(0, 0, 0, 1);

            // discard; //UNCOMMENTING DISCARD MAKES IT CRASH ON AMD??????????
            // Did more research and using discard is actually worse for performance
            // So it doesn't even matter. We'll just use alpha blending instead. ...i wish :<
        }
        else
        {
            outColor = vec4(vec3(newColor), newAlpha);
        }
    }
}