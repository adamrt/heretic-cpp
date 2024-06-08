#pragma once

#include "Mesh.h"
#include "Model.h"
#include "glm/glm.hpp"
#include "glm/matrix.hpp"

class Light : public Model {
public:
    Light(std::shared_ptr<Mesh> _mesh, glm::vec3 _position, glm::vec4 _color)
        : Model(_mesh)
        , color(_color)
    {
        translation = _position;
        scale = glm::vec3(0.3f, 0.3f, 0.3f);
    }

    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    auto render() -> void override;
};
