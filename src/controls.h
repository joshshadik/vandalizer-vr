#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
        glm::vec3 position;
        glm::quat orientation;
    };

    void setCursorPosition(double xpos, double ypos);
    void setMouseButton(int button, int action, int mods);
    void setVRController(int index, const glm::vec3& position, const glm::quat& orientation);

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