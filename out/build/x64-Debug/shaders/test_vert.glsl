#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;

const int MAX_BONES = 100;
layout (location = 0) uniform mat4 gWVP;

uniform mat4 gWorld;
uniform mat4 gBones[MAX_BONES];

void main()
{
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform += gBones[BoneIDs[3]] * Weights[3];

    vec4 PosL = BoneTransform * vec4(pos, 1.0);
    gl_Position = gWVP * PosL;
    TexCoord0 = texCoord;
    vec4 NormalL = BoneTransform * vec4(normal, 0.0);
    Normal0 = (NormalL).xyz;
    WorldPos0 = (PosL).xyz;
    //Normal0 = NormalL;
}