#ifndef MESH_H
#define MESH_H

#include "Math/Vec.h"
#include <vector>

enum class Primitive { Triangles, Lines, Points };

/**
 * Mesh class to manage vertex data and rendering of 3D objects.
 * It encapsulates the creation of vertex buffers and vertex array objects (VAOs) for efficient
 * rendering. The Mesh can be drawn using different primitive types (triangles, lines, points).
 */
class Mesh {
public:
    Mesh() = default;
    explicit Mesh(const std::vector<float>& vertices, Primitive primitive = Primitive::Triangles);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    /**
     * Draw the mesh using the currently bound shader program. This will bind the VAO and issue a
     * draw call based on the primitive type and vertex count. */
    void draw() const;

private:
    uint32_t _vao = 0; // Vertex Array Object ID
    uint32_t _vbo = 0; // Vertex Buffer Object ID
    int _vertexCount = 0;
    Primitive _primitive { Primitive::Triangles };
};

namespace MeshFactory {

Mesh createSphere(float radius, int rings = 16, int segments = 24);
Mesh createBox(const Vec3<float>& halfSize);

} // namespace MeshFactory

#endif // MESH_H
