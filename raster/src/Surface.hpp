#pragma once

#include <glm/glm.hpp>
// collected surface appearance parameters
struct Surface {
    glm::vec3 ambient;   // ambient color
    glm::vec3 diffuse;   // diffuse color
    glm::vec3 specular;  // specular color
    float e;        // specular coefficient

    Surface() : ambient(0, 0, 0), diffuse(1, 1, 1), specular(0, 0, 0), e(0) {}
};
