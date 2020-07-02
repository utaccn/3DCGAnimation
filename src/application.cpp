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
        , robot("resources/RIGING_MODEL_04.obj")
        , environment("resources/only_houses.obj")
        , just_floor("resources/floor.obj")
        , m_texture("resources/checkerboard.png")
        , trees_head("resources/trees_head.obj")
        , trunks("resources/trunks.obj")
        , test("resources/attempt.dae")
        , texToon("resources/zio.jpg")
        , grass("resources/grass1.png")
        , m_camera { &m_window, glm::vec3(2.f, 2.0f, -2.f), -glm::vec3(2.f, 2.0f, -2.f) }//glm::vec3(1.f, 1.0f, 1.f), -glm::vec3(1.f, 1.f,1.f) }
        , cameraLight { &m_window, glm::vec3(30.f, 30.0f, -14.f), -glm::vec3(30.f, 30.0f, -14.f) }//glm::vec3(7.f, 13.0f, -18.f), -glm::vec3(7.f, 13.0f, -18.f) }
        
    {
        m_camera.setUserInteraction(true);
        cameraLight.setUserInteraction(false);
        // === Create Shadow Texture ===

        glCreateTextures(GL_TEXTURE_2D, 1, &texShadow);
        glTextureStorage2D(texShadow, 1, GL_DEPTH_COMPONENT32F, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);

        // Set behaviour for when texture coordinates are outside the [0, 1] range.
        glTextureParameteri(texShadow, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texShadow, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set interpolation for texture sampling (GL_NEAREST for no interpolation).
        glTextureParameteri(texShadow, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texShadow, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // === Create framebuffer for extra texture ===

        glCreateFramebuffers(1, &framebuffer);
        glNamedFramebufferTexture(framebuffer, GL_DEPTH_ATTACHMENT, texShadow, 0);

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

            ShaderBuilder toonBuilder;
            toonBuilder.addStage(GL_VERTEX_SHADER, "shaders/toon_vert.glsl");
            toonBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/toon_frag.glsl");
            toonShader = toonBuilder.build();

            ShaderBuilder x_rayBuilder;
            x_rayBuilder.addStage(GL_VERTEX_SHADER, "shaders/x_ray_vert.glsl");
            x_rayBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/x_ray_frag.glsl");
            x_ray = x_rayBuilder.build();

            ShaderBuilder floorBuilder;
            floorBuilder.addStage(GL_VERTEX_SHADER, "shaders/floor_vert.glsl");
            floorBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/floor_frag.glsl");
            floorShader = floorBuilder.build();

            ShaderBuilder trees_headBuilder;
            trees_headBuilder.addStage(GL_VERTEX_SHADER, "shaders/trees_head_vert.glsl");
            trees_headBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/trees_head_frag.glsl");
            trees_headShader = trees_headBuilder.build();

            ShaderBuilder trunksBuilder;
            trunksBuilder.addStage(GL_VERTEX_SHADER, "shaders/trunk_vert.glsl");
            trunksBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/trunk_frag.glsl");
            trunkShader = trunksBuilder.build();


            ShaderBuilder testBuilder;
            testBuilder.addStage(GL_VERTEX_SHADER, "shaders/test_vert.glsl");
            testBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/test_frag.glsl");
            testShader = testBuilder.build();

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....

        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    GLuint framebuffer;
    GLuint texShadow;
    const int SHADOWTEX_WIDTH = 1024;
    const int SHADOWTEX_HEIGHT = 1024;

    void update()
    {
        // This is your game loop
        // Put your real-time logic and rendering in here
        while (!m_window.shouldClose()) {
            m_window.updateInput();
            {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                glClearDepth(1.0f);
                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);        

                m_shadowShader.bind();
                glViewport(0, 0, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);

                const glm::mat4 lightmvp = m_projectionMatrix * cameraLight.viewMatrix()*m_modelMatrix; // Assume model matrix is identity.
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp));

                environment.draw();
                m_mesh.draw();
                trees_head.draw();
                trunks.draw();
                test.draw();
                //Move model matrix to render shadow map for robot model
                const glm::mat4 robotmodelMatrix = glm::translate(m_modelMatrix, glm::vec3(2.0, 0.3, 0.0));
                const glm::mat4 robotlightmvp = m_projectionMatrix * cameraLight.viewMatrix() *robotmodelMatrix;
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(robotlightmvp));
                robot.draw();

                // Unbind the off-screen framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

            }
            if (x_shader == 0){
                m_defaultShader.bind();
            }
            else {
                x_ray.bind();
                texToon.bind(GL_TEXTURE1);
                glUniform1i(10, 1);
            }
            
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));
            const glm::mat4 mvpMatrix = m_projectionMatrix * m_camera.viewMatrix()* m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            //Don't know if this should go here <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            //glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
            //glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
            const glm::mat4 lightmvp = m_projectionMatrix * cameraLight.viewMatrix(); // Assume model matrix is identity.
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            const glm::vec3 lightPos = cameraLight.cameraPos();
            glUniform3fv(4, 1, glm::value_ptr(lightPos));

            // Bind the shadow map to texture slot 0
            GLuint texture_unit = 0;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            glUniform1i(8, texture_unit);
            glViewport(0, 0, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);

            // Clear the framebuffer to black and depth to maximum value
            glClearDepth(1.0f);
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            environment.draw();

            testShader.bind();
            glm::mat4 newMM = glm::scale(m_modelMatrix, glm::vec3(0.1, 0.1, 0.1));
            newMM = glm::translate(newMM, glm::vec3(0, 2, 0));
            newMM = glm::rotate(newMM, glm::radians(90.f), glm::vec3(1, 0, 0));
            const glm::mat4 mvpMatrixMM = m_projectionMatrix * m_camera.viewMatrix() * newMM;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixMM));

            test.draw();
            if (firstPerson == false) {
                glm::vec3 newCameraPos = glm::vec3(2., 1., 2.);
                /*glm::vec3 charPos = m_camera.cameraPos() + glm::vec3(3.0) * (m_camera.getTarget() - m_camera.cameraPos());
                glm::mat4 charLocation = glm::translate(m_modelMatrix, charPos);
                const glm::mat4 mvpMMatrix = m_projectionMatrix * m_camera.viewMatrix() * charLocation;
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMMatrix));*/
                glm::mat4 lucAt = glm::lookAt(newCameraPos, glm::vec3(0., 0., 0.), glm::vec3(0, 1, 0));
                const glm::mat4 mvpMMatrix = m_projectionMatrix * lucAt * m_modelMatrix;
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMMatrix));
                m_mesh.draw();
            }
            else { m_mesh.draw(); }
            if (x_shader == 0) {
                trees_headShader.bind();
            }
            else {
                x_ray.bind();
                texToon.bind(GL_TEXTURE1);
                glUniform1i(10, 1);
            }
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform1i(8, 0);
            trees_head.draw();

            if (x_shader == 0) {
                trunkShader.bind();
            }
            else {
                x_ray.bind();
                texToon.bind(GL_TEXTURE1);
                glUniform1i(10, 1);
            }
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform1i(8, 0);
            trunks.draw();

            
            floorShader.bind();
          //  if (just_floor.hasTextureCoords()) {
                grass.bind(GL_TEXTURE2);
                glUniform1i(9, 2);
            //}
                const glm::mat4 mvpMatrixF = m_projectionMatrix * m_camera.viewMatrix()* m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixF));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix* cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            just_floor.draw();

            //Robot model with Toon shading
            toonShader.bind();
            const glm::mat4 robotmodelMatrix = glm::translate(m_modelMatrix, glm::vec3(2.0, 0.3, 0.0));
            const glm::mat3 robotNormalModelMatrix = glm::inverseTranspose(glm::mat3(robotmodelMatrix));
            const glm::mat4 robotmvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * robotmodelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(robotmvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(robotmodelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(robotNormalModelMatrix));
            glUniform3fv(3, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(m_camera.cameraPos()));
            robot.draw();
            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &texShadow);
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    int onKeyPressed(int key, int mods)
    {
        if (key == 88 && x_shader==0) {
            x_shader = 1;
        }
        else if (key == 88 && x_shader == 1) {
            x_shader = 0;
        }

        if (key == 49 && firstPerson == true) {
            firstPerson = false;
        }
        else if (key == 49 && firstPerson == false) {
            firstPerson = true;
        }

        std::cout << "Key pressed: " << key << std::endl;
           return key;
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
        }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        std::cout << "cameraX:" << m_camera.cameraPos().x << "cameraY:" << m_camera.cameraPos().y << "cameraZ:" << m_camera.cameraPos().z << std::endl;
        m_camera.setUserInteraction(true);
        cameraLight.setUserInteraction(false);
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
    Shader toonShader;
    Shader x_ray;
    Shader floorShader;
    Shader trees_headShader;
    Shader trunkShader;
    Shader testShader;

    int x_shader = 0;
    int firstPerson = true;

    Mesh robot;
    Mesh m_mesh;
    Mesh environment;
    Mesh just_floor;
    Mesh trees_head;
    Mesh trunks;
    Mesh test;

    Texture m_texture;
    Texture texToon;
    Texture grass;

    // Projection and view matrices for you to fill in and use
    //glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 300.0f);
    glm::mat4 m_projectionMatrix = glm::perspective(glm::pi<float>() / 4.0f, 1.0f, 0.1f, 500.0f);  
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };
};

int main()
{
    Application app;
    app.update();

    return 0;
}
