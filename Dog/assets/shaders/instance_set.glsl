// -------------------------------------------------------------------
// Instances ---------------------------------------------------------
struct Instance {
    mat4 modelMatrix;    // 64 bytes
    mat4 normalMatrix;   // 64 bytes
    vec4 boundingSphere; // 16 bytes (center, radius)
    uint textureIndex;   // 4  bytes
    uint charIndex;      // 4  bytes (if it's NL_INVALID_FONT_INDEX it's not a char)
    uint animationIndex; // 4  bytes
    uint meshIndex;      // 4  bytes
    uint followCamera;   // 4  bytes (0 or 1)

    // 4  bytes padding
    // 4  bytes padding
    // 4  bytes padding
};

layout(std430, set = 1, binding = 0) buffer readonly InstanceDataBuffer {
    Instance instances[NL_MAX_INSTANCES];
} instanceData;

struct MeshData
{
    uint indexCount;   // 4 bytes
    uint firstIndex;   // 4 bytes
    int  vertexOffset; // 4 bytes
    // no additional padding needed
};

// NL_MAX_INSTANCES for the size here isn't fully accurate
// come back to change this later perhaps
layout(std430, set = 1, binding = 1) buffer readonly MeshDataBuffer {
    MeshData meshes[NL_MAX_INSTANCES];
} meshData;
