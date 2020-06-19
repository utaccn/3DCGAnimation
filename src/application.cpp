//#include "Image.h"
#include "window.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include "disable_all_warnings.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

DISABLE_WARNINGS_PUSH()
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <functional>
#include <iostream>
#include <vector>

class Application {
public:
    Application()
        : m_window(glm::ivec2(1024, 1024), "Final Project", false)
        , m_mesh("resources/dragon.obj")
        , environment("resources/environment_01.obj")
        , m_texture("resources/checkerboard.png")
        , m_camera(&m_window)
        , cameraLight{ &m_window, glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(5.0f, 5.0f, 5.0f) }

    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods);
        });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods);

        });

        try {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, "shaders/shadow_vert.glsl");
            m_shadowShader = shadowBuilder.build();

            ShaderBuilder environmentBuilder;
            environmentBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert_environment.glsl");
            environmentBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag_environment.glsl");
            m_environmentShader = environmentBuilder.build();


            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....


        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }

    }

    void update()
    {
        // === Create Shadow Texture ===
        GLuint texShadow;
        const int SHADOWTEX_WIDTH = 1024;
        const int SHADOWTEX_HEIGHT = 1024;
        glCreateTextures(GL_TEXTURE_2D, 1, &texShadow);
        glTextureStorage2D(texShadow, 1, GL_DEPTH_COMPONENT32F, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);
        // Set behaviour for when texture coordinates are outside the [0, 1] range.
        glTextureParameteri(texShadow, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texShadow, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Set interpolation for texture sampling (GL_NEAREST for no interpolation).
        glTextureParameteri(texShadow, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texShadow, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // === Create framebuffer for extra texture ===
        GLuint framebuffer;
        glCreateFramebuffers(1, &framebuffer);
        glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, texShadow, 0);

        // This is your game loop
        // Put your real-time logic and rendering in here
        while (!m_window.shouldClose()) {
            m_window.updateInput();
            {
                // Clear the screen
                glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                // Bind the off-screen framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                // Clear the shadow map and set needed options
                glClearDepth(1.0f);
                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

                // Bind the shader
                m_shadowShader.bind();
                // Set viewport size
                glViewport(0, 0, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);

                //const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;            
                //My camera Light for shadow !!!!!!!
                const glm::mat4 lightmvp = m_projectionMatrix * cameraLight.viewMatrix()*m_modelMatrix; // Assume model matrix is identity.
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp));
                environment.draw();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            m_environmentShader.bind();
            //m_defaultShader.bind();
            glm::mat4 mvpMatrix = m_projectionMatrix * m_camera.viewMatrix() *m_modelMatrix;
            //const glm::vec3 lightPos = cameraLight.cameraPos();
            //glUniform3fv(4, 1, glm::value_ptr(lightPos));
            const glm::mat4 mvp = m_projectionMatrix * m_camera.viewMatrix(); // Assume model matrix is identity.
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));

            //?????????????????????????????????????????//

            /*
            GLuint texture_unit = 4;
            glActiveTexture(GL_TEXTURE0+4);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            glUniform1i(9, texture_unit);
            */
            const glm::mat4 lightmvp = m_projectionMatrix * cameraLight.viewMatrix(); // Assume model matrix is identity.
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp));
            const glm::vec3 lightPos = cameraLight.cameraPos();
            glUniform3fv(4, 1, glm::value_ptr(lightPos));
/*
            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            //Move character according to camera position
            glm::mat4 newModelMatrix = glm::translate(m_modelMatrix, glm::vec3(-0.5, -0.5, -0.5) + m_camera.cameraPos());
            //          newModelMatrix = glm::rotate(newModelMatrix, -glm::radians(m_camera.rotationX()), glm::vec3(1, 0, 0));
            //          newModelMatrix = glm::rotate(newModelMatrix, glm::radians(m_camera.rotationY()), glm::vec3(0, 1, 0));
            glm::mat3 newnormalModelMatrix = glm::inverseTranspose(glm::mat3(newModelMatrix));
            glm::mat4 newmvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * newModelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(newmvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(newModelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(newnormalModelMatrix));
            if (m_mesh.hasTextureCoords()) {
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(3, 0);
                glUniform1i(4, GL_TRUE);
            }
            else {
                glUniform1i(4, GL_FALSE);
            }

            m_mesh.draw();

            m_environmentShader.bind();
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
            glUniform3fv(1, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));

            /*            if (m_mesh.hasTextureCoords()) {
                            m_texture.bind(GL_TEXTURE0);
                            glUniform1i(3, 0);
                            glUniform1i(4, GL_TRUE);
                        }
                        else {
                            glUniform1i(4, GL_FALSE);
                        }*/
            GLuint texture_unit = 0;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            glUniform1i(9, texture_unit);

            //environment.draw();

            glClearDepth(1.0f);
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            environment.draw();
            
            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteTextures(1, &texShadow);
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos)
    {
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
  //m_viewMatrix = glm::lookAt(glm::vec3(cursorPos.x / 1024, cursorPos.y / 1024, -1), glm::vec3(0), glm::vec3(0, 1, 0));
        }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        m_camera.setUserInteraction(true);
        std::cout << "Pressed mouse button: " << button << std::endl;
       
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {
        std::cout << "Released mouse button: " << button << std::endl;
    }

private:
    Window m_window;
    Camera m_camera;
    Camera cameraLight;
    
    // Shader for default rendering and for depth rendering    
    Shader m_defaultShader;
    Shader m_shadowShader;
    Shader m_environmentShader;

    Mesh m_mesh;
    Mesh environment;
    Texture m_texture;

    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };


};

int main()
{
    Application app;
    app.update();

    return 0;
}
