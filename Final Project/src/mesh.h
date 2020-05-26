#pragma once
#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <exception>
#include <filesystem>

struct MeshLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
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

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLsizei m_numIndices { 0 };
    bool m_hasTextureCoords { false };
    GLuint m_ibo { INVALID };
    GLuint m_vbo { INVALID };
    GLuint m_vao { INVALID };
};
