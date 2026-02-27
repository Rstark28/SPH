#include "../../include/UI/Mesh.h"
#include <cmath>
#include <glad/glad.h>

#include "Rules.h"

namespace {
GLenum toGLPrimitive(const Primitive p)
{
    switch (p) {
    case Primitive::Lines:
        return GL_LINES;
    case Primitive::Points:
        return GL_POINTS;
    case Primitive::Triangles:
    default:
        return GL_TRIANGLES;
    }
}

std::vector<float> buildSphereVertices(const float radius, const int rings, const int segments)
{
    std::vector<float> vertices;
    for (int i = 0; i < rings; ++i) {
        const float phi1 = PI * i / rings;
        const float phi2 = PI * (i + 1) / rings;

        const float y1 = radius * std::cos(phi1);
        const float r1 = radius * std::sin(phi1);

        const float y2 = radius * std::cos(phi2);
        const float r2 = radius * std::sin(phi2);

        for (int j = 0; j < segments; ++j) {
            const float theta1 = 2.0f * PI * j / segments;
            const float theta2 = 2.0f * PI * (j + 1) / segments;

            const float x11 = r1 * std::cos(theta1);
            const float z11 = r1 * std::sin(theta1);

            const float x12 = r1 * std::cos(theta2);
            const float z12 = r1 * std::sin(theta2);

            const float x21 = r2 * std::cos(theta1);
            const float z21 = r2 * std::sin(theta1);

            const float x22 = r2 * std::cos(theta2);
            const float z22 = r2 * std::sin(theta2);

            vertices.insert(vertices.end(), { x11, y1, z11 });
            vertices.insert(vertices.end(), { x21, y2, z21 });
            vertices.insert(vertices.end(), { x12, y1, z12 });

            vertices.insert(vertices.end(), { x12, y1, z12 });
            vertices.insert(vertices.end(), { x21, y2, z21 });
            vertices.insert(vertices.end(), { x22, y2, z22 });
        }
    }

    return vertices;
}

} // namespace

Mesh::Mesh(const std::vector<float>& vertices, const Primitive primitive)
    : _primitive(primitive)
{
    _vertexCount = static_cast<int>(vertices.size() / 3);
    if (_vertexCount == 0)
        return;

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    if (_vao)
        glDeleteVertexArrays(1, &_vao);
    if (_vbo)
        glDeleteBuffers(1, &_vbo);
}

Mesh::Mesh(Mesh&& other) noexcept
    : _vao(other._vao)
    , _vbo(other._vbo)
    , _vertexCount(other._vertexCount)
    , _primitive(other._primitive)
{
    other._vao = 0;
    other._vbo = 0;
    other._vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other) {
        if (_vao)
            glDeleteVertexArrays(1, &_vao);
        if (_vbo)
            glDeleteBuffers(1, &_vbo);

        _vao = other._vao;
        _vbo = other._vbo;
        _vertexCount = other._vertexCount;
        _primitive = other._primitive;

        other._vao = 0;
        other._vbo = 0;
        other._vertexCount = 0;
    }
    return *this;
}

void Mesh::draw() const
{
    if (_vao == 0 || _vertexCount == 0)
        return;

    glBindVertexArray(_vao);
    glDrawArrays(toGLPrimitive(_primitive), 0, _vertexCount);
}

Mesh MeshFactory::createSphere(const float radius, const int rings, const int segments)
{
    return Mesh(buildSphereVertices(radius, rings, segments), Primitive::Triangles);
}

Mesh MeshFactory::createBox(const Vec3<float>& halfSize)
{
    const float hx = halfSize[0];
    const float hy = halfSize[1];
    const float hz = halfSize[2];

    const std::vector vertices = {
        // Bottom face
        -hx,
        -hy,
        -hz,
        hx,
        -hy,
        -hz,
        hx,
        -hy,
        -hz,
        hx,
        -hy,
        hz,
        hx,
        -hy,
        hz,
        -hx,
        -hy,
        hz,
        -hx,
        -hy,
        hz,
        -hx,
        -hy,
        -hz,

        // Top face
        -hx,
        hy,
        -hz,
        hx,
        hy,
        -hz,
        hx,
        hy,
        -hz,
        hx,
        hy,
        hz,
        hx,
        hy,
        hz,
        -hx,
        hy,
        hz,
        -hx,
        hy,
        hz,
        -hx,
        hy,
        -hz,

        // Vertical edges
        -hx,
        -hy,
        -hz,
        -hx,
        hy,
        -hz,
        hx,
        -hy,
        -hz,
        hx,
        hy,
        -hz,
        hx,
        -hy,
        hz,
        hx,
        hy,
        hz,
        -hx,
        -hy,
        hz,
        -hx,
        hy,
        hz,
    };

    return Mesh(vertices, Primitive::Lines);
}
