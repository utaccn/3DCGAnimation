#pragma once
#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <exception>
#include <filesystem>
#include <assimp\matrix4x4.h>
#include <assimp\anim.h>
#include <assimp\scene.h>

struct MeshLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct VertexBoneData
{
    int IDs[4];
    float Weights[4];
};

struct BoneInfo
{
    aiMatrix4x4 BoneOffset;
    aiMatrix4x4 FinalTransformation;
};


class Mesh {
public:
    Mesh(std::filesystem::path filePath);
    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&);
    ~Mesh();

    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&);

    bool hasTextureCoords() const;

    // Bind VAO and call glDrawElements.
    void draw();

private:
    void moveInto(Mesh&&);
    void freeGpuMemory();


    /////////////////////////////////////////////////////////////////////////////////////////
    
    void BoneTransform(float TimeInSeconds, std::vector<aiMatrix4x4>& Transforms);

    int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);

    int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform);

    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

    void LoadBones(int MeshIndex);
    
    /////////////////////////////////////////////////////////////////////////////////////////



private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLsizei m_numIndices { 0 };
    bool m_hasTextureCoords { false };
    GLuint m_ibo { INVALID };
    GLuint m_vbo { INVALID };
    GLuint m_vao { INVALID };
};
