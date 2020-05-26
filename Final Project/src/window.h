#pragma once
#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
DISABLE_WARNINGS_POP()
#include <functional>
#include <string_view>
#include <vector>

struct GLFWwindow;

class Window {
public:
    Window(const glm::ivec2& resolution, std::string_view title, bool resizeable = true, bool legacyOpenGL = false);
    Window(const Window&) = delete;
    ~Window();

    void close(); // Set shouldClose() to true.
    bool shouldClose(); // Whether window should close (close() was called or user clicked the close button).

    void updateInput();
    void swapBuffers(); // Swap the front/back buffer.

    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    void registerKeyCallback(KeyCallback&&);
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    void registerMouseButtonCallback(MouseButtonCallback&&);
    using MouseMoveCallback = std::function<void(const glm::dvec2& position)>;
    void registerMouseMoveCallback(MouseMoveCallback&&);
    using PostInputUpdateCallback = std::function<void()>;
    void registerPostInputUpdateCallback(PostInputUpdateCallback&&);
    using WindowResizeCallback = std::function<void(const glm::ivec2& size)>;
    void registerWindowResizeCallback(WindowResizeCallback&&);

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void setCursorPos(const glm::dvec2& newPos);
    glm::dvec2 getCursorPos() const;
    glm::vec2 getCursorPixel() const;

    // Hides mouse and prevents it from going out of the window.
    // Usefull for a first person camera.
    void setMouseCapture(bool capture);

    glm::ivec2 getResolution() const;

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);

private:
    glm::ivec2 m_resolution;
    GLFWwindow* m_pWindow { nullptr };

    std::vector<KeyCallback> m_keyCallbacks;
    std::vector<MouseButtonCallback> m_mouseButtonCallbacks;
    std::vector<MouseMoveCallback> m_mouseMoveCallbacks;
    std::vector<PostInputUpdateCallback> m_postInputUpdateCallbacks;
    std::vector<WindowResizeCallback> m_windowResizeCallbacks;
};
