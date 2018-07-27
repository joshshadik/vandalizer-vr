#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/constants.hpp>

// glm::vec3 saturate(glm::vec3 v)
// {
//     return glm::clamp(v, 0, 1);
// }

glm::vec3 Hue(float h)
{
    float r = glm::abs(h * 6 - 3) - 1;
    float g = 2 - glm::abs(h * 6 - 2);
    float b = 2 - glm::abs(h * 6 - 4);
    return glm::saturate(glm::vec3(r,g,b));
}

glm::vec3 HSVtoRGB(const glm::vec3& hsv)
{
    return glm::vec3(((Hue(hsv.x) - glm::vec3(1.0f, 1.0f, 1.0f)) * hsv.y + glm::vec3(1.0f, 1.0f, 1.0f)) * hsv.z);
}