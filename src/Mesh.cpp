#include "Mesh.h"

Mesh::Mesh(const std::vector<float> vertices, const std::vector<uint16_t> indices)
{
    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = sg_range { vertices.data(), vertices.size() * sizeof(float) };
    vbuf_desc.label = "vertex-buffer";
    vertex_buffer = sg_make_buffer(&vbuf_desc);

    sg_buffer_desc ibuf_desc = {};
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuf_desc.data = sg_range { indices.data(), indices.size() * sizeof(uint16_t) };
    ibuf_desc.label = "index-buffer";
    index_buffer = sg_make_buffer(&ibuf_desc);

    num_indices = indices.size();
}
