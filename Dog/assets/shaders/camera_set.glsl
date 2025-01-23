// -------------------------------------------------------------------
// Camera -----------------------------------------------------------
layout(set = 0, binding = 0) uniform MatrixUniforms {
    mat4 projectionView;
    mat4 projection;
    mat4 view;
    vec3 directionToLight;
} uniforms;