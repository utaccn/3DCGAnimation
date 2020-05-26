#include "window.h"
#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <GL/glew.h> // Include before glfw3
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>

void glfwErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW error code: " << error << std::endl;
    std::cerr << description << std::endl;
    exit(1);
}

// OpenGL debug callback
void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        std::cerr << "OpenGL: " << message << std::endl;
    }
}

Window::Window(const glm::ivec2& resolution, std::string_view title, bool resizeable, bool legacyOpenGL)
    : m_resolution(resolution)
{
    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW" << std::endl;
        exit(1);
    }

    if (!legacyOpenGL) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
#ifndef NDEBUG // Automatically defined by CMake when compiling in Release/MinSizeRel mode.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, resizeable ? GL_TRUE : GL_FALSE);

    // std::string_view does not guarantee that the string contains a terminator character.
    std::string titleString { title };
    glfwSetErrorCallback(glfwErrorCallback);
    m_pWindow = glfwCreateWindow(resolution.x, resolution.y, titleString.c_str(), nullptr, nullptr);
    if (m_pWindow == nullptr) {
        glfwTerminate();
        std::cerr << "Could not create GLFW window" << std::endl;
        exit(1);
    }

    glfwMakeContextCurrent(m_pWindow);
    glfwSwapInterval(1); // Wait for window to display frame before rendering the next frame.

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        std::cerr << "Could not initialize GLEW" << std::endl;
        exit(1);
    }

#ifndef NDEBUG
    glDebugMessageCallback(glDebugCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    glfwSetWindowUserPointer(m_pWindow, this);

    glfwSetKeyCallback(m_pWindow, keyCallback);
    glfwSetMouseButtonCallback(m_pWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(m_pWindow, mouseMoveCallback);
    glfwSetWindowSizeCallback(m_pWindow, windowSizeCallback);
}

Window::~Window()
{
    if (m_pWindow) {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }
}

void Window::close()
{
    glfwSetWindowShouldClose(m_pWindow, 1);
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(m_pWindow) != 0;
}

void Window::updateInput()
{
    glfwPollEvents();

    for (const auto& callback : m_postInputUpdateCallbacks) {
        callback();
    }
}

void Window::swapBuffers()
{
    //glm::ivec2 resolution;
    //glfwGetFramebufferSize(m_pWindow, &resolution.x, &resolution.y);
    //glViewport(0, 0, resolution.x, resolution.y);

    glfwSwapBuffers(m_pWindow);
}

void Window::registerKeyCallback(KeyCallback&& callback)
{
    m_keyCallbacks.push_back(std::move(callback));
}

void Window::registerMouseButtonCallback(MouseButtonCallback&& callback)
{
    m_mouseButtonCallbacks.push_back(std::move(callback));
}

void Window::registerMouseMoveCallback(MouseMoveCallback&& callback)
{
    m_mouseMoveCallbacks.push_back(std::move(callback));
}

void Window::registerPostInputUpdateCallback(PostInputUpdateCallback&& callback)
{
    m_postInputUpdateCallbacks.push_back(callback);
}

void Window::registerWindowResizeCallback(WindowResizeCallback&& callback)
{
    m_windowResizeCallbacks.push_back(std::move(callback));
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    for (const auto& callback : thisWindow->m_keyCallbacks)
        callback(key, scancode, action, mods);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    for (const auto& callback : thisWindow->m_mouseButtonCallbacks)
        callback(button, action, mods);
}

void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    Window* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    for (const auto& callback : thisWindow->m_mouseMoveCallbacks)
        callback(glm::dvec2(xpos, ypos));
}

void Window::windowSizeCallback(GLFWwindow* window, int width, int height)
{
    Window* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    thisWindow->m_resolution = glm::ivec2 { width, height };

    for (const auto& callback : thisWindow->m_windowResizeCallbacks)
        callback(glm::ivec2(width, height));
}

bool Window::isKeyPressed(int key) const
{
    return glfwGetKey(m_pWindow, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const
{
    return glfwGetMouseButton(m_pWindow, button) == GLFW_PRESS;
}

void Window::setCursorPos(const glm::dvec2& newPos)
{
    glfwSetCursorPos(m_pWindow, newPos.x, newPos.y);
}

glm::dvec2 Window::getCursorPos() const
{
    double x, y;
    glfwGetCursorPos(m_pWindow, &x, &y);
    return glm::dvec2(x, y);
}

glm::vec2 Window::getCursorPixel() const
{
    // https://stackoverflow.com/questions/45796287/screen-coordinates-to-world-coordinates
    // Coordinates returned by glfwGetCursorPos are in screen coordinates which may not map 1:1 to
    // pixel coordinates on some machines (e.g. with resolution scaling).
    glm::ivec2 screenSize;
    glfwGetWindowSize(m_pWindow, &screenSize.x, &screenSize.y);
    glm::ivec2 framebufferSize;
    glfwGetFramebufferSize(m_pWindow, &framebufferSize.x, &framebufferSize.y);

    double xpos, ypos;
    glfwGetCursorPos(m_pWindow, &xpos, &ypos);
    const glm::vec2 screenPos { xpos, ypos };
    glm::vec2 pixelPos = screenPos * glm::vec2(framebufferSize) / glm::vec2(screenSize); // float division
    pixelPos += glm::vec2(0.5f); // Shift to GL center convention.
    return glm::vec2(pixelPos.x, static_cast<float>(framebufferSize.y) - pixelPos.y - 1);
}

glm::ivec2 Window::getResolution() const
{
    return m_resolution;
}

void Window::setMouseCapture(bool capture)
{
    if (capture) {
        glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    glfwPollEvents();
}
