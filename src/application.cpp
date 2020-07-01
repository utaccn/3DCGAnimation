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
class quad {
public:
    quad() {
        GLfloat vertices[] = { // format = x, y, z, u, v
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
        };
        glGenVertexArrays(1, &vao); // vao saves state of array buffer, element array, etc
        glGenBuffers(1, &vbo); // vbo stores vertex data
        GLint curr_vao; // original state
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &curr_vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(curr_vao);
    }

    ~quad() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    GLuint vao, vbo;
};
class Application {
public:
    Application()
        : m_window(glm::ivec2(1024, 1024), "Final Project", false)
        , m_mesh("resources/chara.obj")
        , robot("resources/RIGING_MODEL_04.obj")
        , environment("resources/only_houses.obj")
        , just_floor("resources/floor.obj")
        , m_texture("resources/checkerboard.png")
        , trees_head("resources/trees_head.obj")
        , trunks("resources/trunks.obj")
        , texToon("resources/zio.jpg")
        , grass("resources/grass1.png")
        , m_camera { &m_window, glm::vec3(2.,2.0f, -2.f), -glm::vec3(2.f, 2.0f, -2.f) }
        , cameraLight { &m_window, glm::vec3(30.f, 30.0f, -14.f), -glm::vec3(30.f, 30.0f, -14.f) }
        , minimapCamera{ &m_window, glm::vec3(1.f, 45.0f, 0.f), -glm::vec3(1.f,45.0f, 0.f) }

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

        //MINIMAP
        glCreateTextures(GL_TEXTURE_2D, 1, &minimap);
        glBindTexture(GL_TEXTURE_2D, minimap);
        glTextureParameteri(minimap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(minimap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(minimap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(minimap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glCreateFramebuffers(1, &framebuffo);
        glNamedFramebufferTexture(framebuffo, GL_COLOR_ATTACHMENT0, minimap, 0);



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

            ShaderBuilder miniBuilder;
            miniBuilder.addStage(GL_VERTEX_SHADER, "shaders/minimap_vert.glsl");
            miniBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/minimap_frag.glsl");
            minimapShader = miniBuilder.build();

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....


        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
        float vertexx[] = {
             1.f,  1.f, 0.0f,  // top right
             1.f, -1.f, 0.0f,  // bottom right
            -1.f, -1.f, 0.0f,  // bottom left
            -1.f,  1.f, 0.0f   // top left 
        };
        unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
        };


        glGenVertexArrays(1, &VAOO);
        glGenBuffers(1, &VBOO);
        glGenBuffers(1, &EBOO);
        glBindVertexArray(VAOO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexx), vertexx, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

    }
    unsigned int fbo;
    unsigned int VAOO, VBOO, EBOO;

    GLuint framebuffer;
    GLuint texShadow;
    GLuint framebuffo;
    GLuint minimap;
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
                //Move model matrix to render shadow map for robot model
                const glm::mat4 robotmodelMatrix = glm::translate(m_modelMatrix, glm::vec3(2.0, 0.3, 0.0));
                const glm::mat4 robotlightmvp = m_projectionMatrix * cameraLight.viewMatrix() *robotmodelMatrix;
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(robotlightmvp));
                robot.draw();

                // Unbind the off-screen framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

            }
            glClearDepth(1.0f);
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, 256, 256);
            if (x_shader == 0){
                m_defaultShader.bind();
            }
            else {
                x_ray.bind();
                texToon.bind(GL_TEXTURE1);
                glUniform1i(10, 1);
            }     
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));
            const glm::mat4 mvpMatrix = m_projectionMatrix * minimapCamera.viewMatrix()* m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            const glm::mat4 lightmvp = m_projectionMatrix * cameraLight.viewMatrix(); // Assume model matrix is identity.
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp));
            glUniform3fv(5, 1, glm::value_ptr(minimapCamera.cameraPos()));
            const glm::vec3 lightPos = cameraLight.cameraPos();
            glUniform3fv(4, 1, glm::value_ptr(lightPos));

            // Bind the shadow map to texture slot 0
            GLuint texture_unit = 0;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            glUniform1i(8, texture_unit);
            environment.draw();
            
            if (firstPerson == false) {
                glm::vec3 newCameraPos = glm::vec3(2., 1., 2.);
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
            glUniform3fv(5, 1, glm::value_ptr(minimapCamera.cameraPos()));
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
            glUniform3fv(5, 1, glm::value_ptr(minimapCamera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform1i(8, 0);
            trunks.draw();
            
            floorShader.bind();
          //  if (just_floor.hasTextureCoords()) {
                grass.bind(GL_TEXTURE2);
                glUniform1i(9, 2);
            //}
                const glm::mat4 mvpMatrixF = m_projectionMatrix * minimapCamera.viewMatrix()* m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixF));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix* cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(minimapCamera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            just_floor.draw();
            
            //Robot model with Toon shading
            toonShader.bind();
            const glm::mat4 robotmodelMatrix = glm::translate(m_modelMatrix, glm::vec3(2.0, 0.3, 0.0));
            const glm::mat3 robotNormalModelMatrix = glm::inverseTranspose(glm::mat3(robotmodelMatrix));
            const glm::mat4 robotmvpMatrix = m_projectionMatrix * minimapCamera.viewMatrix() * robotmodelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(robotmvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(robotmodelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(robotNormalModelMatrix));
            glUniform3fv(3, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(minimapCamera.cameraPos()));
            robot.draw();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, 1024, 1024);
            if (x_shader == 0) {
                m_defaultShader.bind();
            }
            else {
                x_ray.bind();
                texToon.bind(GL_TEXTURE1);
                glUniform1i(10, 1);
            }


            const glm::mat3 normalModelMatrix1 = glm::inverseTranspose(glm::mat3(m_modelMatrix));
            const glm::mat4 mvpMatrix1 = m_projectionMatrix * m_camera.viewMatrix() * m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix1));
            const glm::mat4 lightmvp1 = m_projectionMatrix * cameraLight.viewMatrix(); // Assume model matrix is identity.
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(lightmvp1));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            const glm::vec3 lightPos1 = cameraLight.cameraPos();
            glUniform3fv(4, 1, glm::value_ptr(lightPos1));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            glUniform1i(8, 0);
            environment.draw();

            if (firstPerson == false) {
                glm::vec3 newCameraPos = glm::vec3(2., 1., 2.);
                glm::mat4 lucAt1 = glm::lookAt(newCameraPos, glm::vec3(0., 0., 0.), glm::vec3(0, 1, 0));
                const glm::mat4 mvpMMatrix = m_projectionMatrix * lucAt1 * m_modelMatrix;
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
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix1));
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
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix1));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform1i(8, 0);
            trunks.draw();

            floorShader.bind();
            grass.bind(GL_TEXTURE2);
            glUniform1i(9, 2);
            //}
            const glm::mat4 mvpMatrixF1 = m_projectionMatrix * m_camera.viewMatrix() * m_modelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrixF1));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * cameraLight.viewMatrix()));
            glUniform3fv(5, 1, glm::value_ptr(m_camera.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(cameraLight.cameraPos()));
            just_floor.draw();

            //Robot model with Toon shading
            toonShader.bind();
            const glm::mat4 robotmodelMatrix1 = glm::translate(m_modelMatrix, glm::vec3(2.0, 0.3, 0.0));
            const glm::mat3 robotNormalModelMatrix1 = glm::inverseTranspose(glm::mat3(robotmodelMatrix));
            const glm::mat4 robotmvpMatrix1 = m_projectionMatrix * m_camera.viewMatrix() * robotmodelMatrix;
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(robotmvpMatrix1));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(robotmodelMatrix1));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(robotNormalModelMatrix1));
            glUniform3fv(3, 1, glm::value_ptr(cameraLight.cameraPos()));
            glUniform3fv(4, 1, glm::value_ptr(m_camera.cameraPos()));
            robot.draw();

            glViewport(0, 0, 256, 256);
            quad q;
            minimapShader.bind();
            glBindVertexArray(q.vao);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, minimap);
            glUniform1i(10, 3);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            /*
            //MINIMAP
            minimapShader.bind();
            const glm::mat4 mvpMini = m_projectionMatrix * m_camera.viewMatrix();// *m_modelMatrix; // Assume model matrix is identity.
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMini));
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, minimap);
            glUniform1i(10, 4);
            glBindVertexArray(VAOO);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/

            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);

        glDeleteVertexArrays(1, &VAOO);
        glDeleteBuffers(1, &VBOO);
        glDeleteBuffers(1, &EBOO);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteFramebuffers(1, &framebuffo);
        glDeleteTextures(1, &texShadow);
        glDeleteTextures(1, &minimap);
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
    Camera minimapCamera;
    
    // Shader for default rendering and for depth rendering    
    Shader m_defaultShader;
    Shader m_shadowShader;
    Shader toonShader;
    Shader x_ray;
    Shader floorShader;
    Shader trees_headShader;
    Shader trunkShader;
    Shader minimapShader;

    int x_shader = 0;
    int firstPerson = true;

    Mesh robot;
    Mesh m_mesh;
    Mesh environment;
    Mesh just_floor;
    Mesh trees_head;
    Mesh trunks;

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
