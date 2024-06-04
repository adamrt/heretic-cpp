#include "Mesh.h"

// pos3 + norm3 + uv2 + color4
const int vertex_size = 12;
Mesh::Mesh(const std::vector<float> vertices)
{
    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = sg_range { vertices.data(), vertices.size() * sizeof(float) };
    vbuf_desc.label = "vertex-buffer";
    vertex_buffer = sg_make_buffer(&vbuf_desc);

    num_indices = vertices.size() / vertex_size;
}
