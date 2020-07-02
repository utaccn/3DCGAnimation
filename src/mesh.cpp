#include "mesh.h"
#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fmt/format.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <gsl/span>
DISABLE_WARNINGS_POP()
#include <iostream>
#include <stack>

static glm::mat4 assimpMatrix(const aiMatrix4x4& m);
static glm::vec3 assimpVec(const aiVector3D& v);
static const aiScene* scene;
static Assimp::Importer importer;

std::vector<VertexBoneData> Bones;

std::map<std::string, int> m_BoneMapping;
std::vector<BoneInfo> BoneInformation;
Mesh::Mesh(std::filesystem::path filePath)
{
    if (!std::filesystem::exists(filePath))
        throw MeshLoadingException(fmt::format("File {} does not exist", filePath.string().c_str()));

    scene = importer.ReadFile(filePath.string().data(), aiProcess_GenSmoothNormals | aiProcess_Triangulate);
    bool Ret = false;
    
    if (scene == nullptr || scene->mRootNode == nullptr /*|| scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE*/) {
        throw MeshLoadingException(fmt::format("Assimp failed to load mesh file {}", filePath.string().c_str()));
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::stack<std::tuple<aiNode*, glm::mat4>> stack;
    stack.push({ scene->mRootNode, assimpMatrix(scene->mRootNode->mTransformation) });

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //std::cout << "Mesh: " << scene->mMeshes[0]->mName.data << " Stack: " << stack.size() << std::endl;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    while (!stack.empty()) {
        auto [node, matrix] = stack.top();
        stack.pop();

        matrix *= assimpMatrix(node->mTransformation);
        const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(matrix));

        for (unsigned i = 0; i < node->mNumMeshes; i++) {
            // Process subMesh
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            
            if (mesh->mNumVertices == 0 || mesh->mNumFaces == 0)
                std::cerr << "Empty mesh encountered" << std::endl;

            // Triangles
            const size_t indexOffset = vertices.size();
            for (unsigned j = 0; j < mesh->mNumFaces; j++) {
                const aiFace& face = mesh->mFaces[j];
                if (face.mNumIndices != 3) {
                    std::cerr << "Found a face which is not a triangle, discarding!" << std::endl;
                }
                auto aiIndices = face.mIndices;
                indices.push_back(static_cast<unsigned>(aiIndices[0] + indexOffset));
                indices.push_back(static_cast<unsigned>(aiIndices[1] + indexOffset));
                indices.push_back(static_cast<unsigned>(aiIndices[2] + indexOffset));
            }

            // Vertices
            for (unsigned j = 0; j < mesh->mNumVertices; j++) {
                const glm::vec3 pos = matrix * glm::vec4(assimpVec(mesh->mVertices[j]), 1.0f);
                const glm::vec3 normal = normalMatrix * assimpVec(mesh->mNormals[j]);
                glm::vec2 texCoord { 0 };
                if (mesh->HasTextureCoords(0)) {
                    texCoord = glm::vec2(assimpVec(mesh->mTextureCoords[0][j]));
                    m_hasTextureCoords = true;
                }
                vertices.push_back(Vertex { pos, normal, texCoord });
            }

            //Bones
            if (mesh->HasBones())            
                Bones.resize(vertices.size());
        }

        for (unsigned i = 0; i < node->mNumChildren; i++) {
            stack.push({ node->mChildren[i], matrix });
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << "Mesh: " << scene->mMeshes[0]->mName.data << " Vertices: " << vertices.size() << " Indices: " << indices.size() << std::endl;
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    importer.FreeScene();

    // Create Element(/Index) Buffer Objects and Vertex Buffer Object.
    glCreateBuffers(1, &m_ibo);
    glNamedBufferStorage(m_ibo, static_cast<GLsizeiptr>(indices.size() * sizeof(decltype(indices)::value_type)), indices.data(), 0);

    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), 0);

    // Bind vertex data to shader inputs using their index (location).
    // These bindings are stored in the Vertex Array Object.
    glCreateVertexArrays(1, &m_vao);

    // The indicies (pointing to vertices) should be read from the index buffer.
    glVertexArrayElementBuffer(m_vao, m_ibo);

    // The position and normal vectors should be retrieved from the specified Vertex Buffer Object.
    // The stride is the distance in bytes between vertices. We use the offset to point to the normals
    // instead of the positions.
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, offsetof(Vertex, pos), sizeof(Vertex));
    glVertexArrayVertexBuffer(m_vao, 1, m_vbo, offsetof(Vertex, normal), sizeof(Vertex));
    glVertexArrayVertexBuffer(m_vao, 2, m_vbo, offsetof(Vertex, texCoord), sizeof(Vertex));
    // Bones
    glVertexArrayVertexBuffer(m_vao, 3, m_vbo, offsetof(VertexBoneData, IDs), sizeof(int));
    // Weights
    glVertexArrayVertexBuffer(m_vao, 4, m_vbo, offsetof(VertexBoneData, Weights), sizeof(float));
    glEnableVertexArrayAttrib(m_vao, 0);
    glEnableVertexArrayAttrib(m_vao, 1);
    glEnableVertexArrayAttrib(m_vao, 2);
    // Bones
    glEnableVertexArrayAttrib(m_vao, 3);
    // Weights
    glEnableVertexArrayAttrib(m_vao, 4);
    m_numIndices = static_cast<GLsizei>(indices.size());
}

Mesh::Mesh(Mesh&& other)
{
    moveInto(std::move(other));
}

Mesh::~Mesh()
{
    freeGpuMemory();
}

Mesh& Mesh::operator=(Mesh&& other)
{
    moveInto(std::move(other));
    return *this;
}

bool Mesh::hasTextureCoords() const
{
    return m_hasTextureCoords;
}

void Mesh::draw()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);
}


void Mesh::moveInto(Mesh&& other)
{
    freeGpuMemory();
    m_numIndices = other.m_numIndices;
    m_hasTextureCoords = other.m_hasTextureCoords;
    m_ibo = other.m_ibo;
    m_vbo = other.m_vbo;
    m_vao = other.m_vao;

    other.m_numIndices = 0;
    other.m_hasTextureCoords = other.m_hasTextureCoords;
    other.m_ibo = INVALID;
    other.m_vbo = INVALID;
    other.m_vao = INVALID;
}

void Mesh::freeGpuMemory()
{
    if (m_vao != INVALID)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo != INVALID)
        glDeleteBuffers(1, &m_vbo);
    if (m_ibo != INVALID)
        glDeleteBuffers(1, &m_ibo);
}

static glm::mat4 assimpMatrix(const aiMatrix4x4& m)
{
    //float values[3][4] = {};
    glm::mat4 matrix;
    matrix[0][0] = m.a1;
    matrix[0][1] = m.b1;
    matrix[0][2] = m.c1;
    matrix[0][3] = m.d1;
    matrix[1][0] = m.a2;
    matrix[1][1] = m.b2;
    matrix[1][2] = m.c2;
    matrix[1][3] = m.d2;
    matrix[2][0] = m.a3;
    matrix[2][1] = m.b3;
    matrix[2][2] = m.c3;
    matrix[2][3] = m.d3;
    matrix[3][0] = m.a4;
    matrix[3][1] = m.b4;
    matrix[3][2] = m.c4;
    matrix[3][3] = m.d4;
    return matrix;
}

static glm::vec3 assimpVec(const aiVector3D& v)
{
    return glm::vec3(v.x, v.y, v.z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
aiMatrix4x4 Mesh::BoneTransform(float TimeInSeconds, std::vector<aiMatrix4x4>& Transforms)
{
   glm::mat4 Identity = glm::mat4{ 1.0f };


    float TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ?
        scene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, scene->mAnimations[0]->mDuration);

    ReadNodeHeirarchy(AnimationTime, scene->mRootNode, Identity);

    Transforms.resize(scene->mMeshes[0]->mNumBones);

    for (int i = 0; i < scene->mMeshes[0]->mNumBones; i++) {
        Transforms[i] = BoneInformation[i].FinalTransformation;
    }
}
void Mesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform)
{
    std::string NodeName(pNode->mName.data);

    const aiAnimation* pAnimation = scene->mAnimations[0];

    aiMatrix4x4 NodeTransformation(pNode->mTransformation);

    for (int i = 0; i < pAnimation->mNumChannels;) {
        const aiNodeAnim* pNodeAnim = //FindNodeAnim(pAnimation, NodeName);
    
    }

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        aiMatrix4x4 ScalingM;
        ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        aiMatrix4x4 RotationM = aiMatrix4x4(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        aiMatrix4x4 TranslationM;
        TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    aiMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        int BoneIndex = m_BoneMapping[NodeName];
        BoneInformation[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation *
            BoneInformation[BoneIndex].BoneOffset;
    }

    for (int i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

void Mesh::LoadBones(int MeshIndex)
{
    const aiMesh* pMesh = scene->mMeshes[0];
    for (int i = 0; i <  pMesh->mNumBones; i++) {
        int BoneIndex = 0;
        std::string BoneName(pMesh->mBones[i]->mName.data);
        
        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            BoneIndex = m_BoneMapping.size();

            //m_NumBones++;
            BoneInfo bi;
            BoneInformation.push_back(bi);
            
            m_BoneMapping.insert(std::pair<std::string, int>(BoneName, BoneIndex));
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }
                
        BoneInformation[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;

        for (int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
            int VertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
            

            for (int i = 0; i < 4; i++) {
                if (Bones[VertexID].Weights[i] == 0.0) {
                    Bones[VertexID].IDs[i] = BoneIndex;
                    Bones[VertexID].Weights[i] = Weight;
                    return;
                }
            }

        }
    }
}










/*#include "Model.h"
#include <fstream>
#include <iostream>
#include <tiny_obj_loader.h>

Model loadModel(std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;

    std::ifstream ifs(path.c_str());

    if (!ifs.is_open()) {
        std::cerr << "Failed to find file: " << path << std::endl;
        exit(1);
    }

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &ifs);

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load object: " << path << std::endl;
        exit(1);
    }

    Model model;

    if (attrib.normals.size() == 0) {
        std::cerr << "Model does not have normal vectors, please re-export with normals." << std::endl;
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                model.vertices.push_back(Vector3f(vx, vy, vz));
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                model.normals.push_back(Vector3f(nx, ny, nz));

                if (attrib.texcoords.size() > 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    model.texCoords.push_back(Vector2f(tx, 1 - ty));
                }

                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }

    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vector3f), model.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(Vector3f), model.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    if (model.texCoords.size() > 0) {
        GLuint tbo;
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(Vector2f), model.texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    return model;
}*/
