#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>

class Controls 
{
public:
    enum Button
    {
        MOUSE_LEFT,
        MOUSE_RIGHT
    };

    struct VRController 
    {
        enum Button
        {
            TRIGGER
        };

        glm::vec3 position;
        glm::quat orientation;

        std::array<float, 4> axis;

        uint32_t pressedFlags;
    };

    void setCursorPosition(double xpos, double ypos);
    void setMouseButton(int button, int action, int mods);
    void setVRController(int index, const glm::vec3& position, const glm::quat& orientation, uint32_t pressedFlags);
    void setVRControllerAxis(int index, int axis, float value);

    bool buttonPressed(Button b);
    bool buttonHeld(Button b);
    bool buttonReleased(Button b);

    const glm::vec2& mousePos() { return _mousePosition; }
    glm::vec2 deltaMousePos();

    const VRController& getVRController(int index);

    void updateStates();

private:
    uint32_t _pressedFlags;
    uint32_t _holdFlags;
    uint32_t _releaseFlags;

    glm::vec2 _mousePosition;
    glm::vec2 _lastMousePosition;

    VRController _vrControllers[2];

};