/*#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <thread>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "renderable.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void toggle_clouds();

const int WindowWidth = 1920;
const int WindowHeight = 1080;
const std::string WindowTitle = "CaribbeanGL";
const float TargetFPS = 60.0f;
const float TargetFrameTime = 1.0f / TargetFPS;
const float SEA_LEVEL_CHANGE = 0.05f;


static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

float lastX = WindowWidth / 2.0f;
float lastY = WindowHeight / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool cloudsEnabled = true;

static void
DrawFloor(unsigned vao, const Shader& shader, unsigned diffuse, unsigned specular) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular);
    float Size = 4.0f;
    for (int i = -2; i < 4; ++i) {
        for (int j = -2; j < 4; ++j) {
            glm::mat4 Model(1.0f);
            Model = glm::translate(Model, glm::vec3(i * Size, -2.0f, j * Size));
            Model = glm::scale(Model, glm::vec3(Size, 0.1f, Size));
            shader.SetModel(Model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}


int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(ErrorCallback);


    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(Window);

    glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback);


    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader Basic("shaders/basic.vert", "shaders/basic.frag");
    Shader ColorShader("shaders/color.vert", "shaders/color.frag");

    float cubeVertices[] = 
    {
        -0.2, -0.2, -0.2,       0.0, 0.0, 0.0,
        +0.2, -0.2, -0.2,       0.0, 0.0, 0.0,
        -0.2, -0.2, +0.2,       0.0, 0.0, 0.0,
        +0.2, -0.2, +0.2,       0.0, 0.0, 0.0,

        -0.2, +0.2, -0.2,       0.0, 0.0, 0.0,
        +0.2, +0.2, -0.2,       0.0, 0.0, 0.0,
        -0.2, +0.2, +0.2,       0.0, 0.0, 0.0,
        +0.2, +0.2, +0.2,       0.0, 0.0, 0.0,
    };

    unsigned int cubeIndices[] = {
        0, 1, 3,
        0, 3, 2,

        4, 6, 7,
        5, 4, 7,

        3, 6, 2,
        6, 3, 7,

        0, 4, 1,
        1, 4, 5,

        6, 0, 2,
        4, 0, 6,

        3, 1, 7,
        7, 1, 5
    };

    Renderable cube(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
    Model Doggo("ki61/1.obj");
    if (!Doggo.Load())
    {
        std::cout << "Failed to load model!\n";
        glfwTerminate();
        return -1;
    }

    glm::mat4 m(1.0f);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.69, 0.86, 0.97, 1.0);


    float FrameStartTime = glfwGetTime();
    float FrameEndTime = glfwGetTime();
    glfwSetKeyCallback(Window, key_callback);
    float dt = FrameEndTime - FrameStartTime;
    float seaLevel = 100;
    float seaLevelChange = SEA_LEVEL_CHANGE;
    while (!glfwWindowShouldClose(Window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        FrameStartTime = glfwGetTime();
        glUseProgram(ColorShader.GetId());



        //Sea
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -23, -13));
        m = glm::scale(m, glm::vec3(700, seaLevel, 400));
        Basic.SetModel(m);
        cube.Render();

        seaLevel += seaLevelChange;
        if (seaLevel > 108) seaLevelChange = -SEA_LEVEL_CHANGE;
        if (seaLevel < 100) seaLevelChange = SEA_LEVEL_CHANGE;

        //Clouds
        if (cloudsEnabled) {
            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2, 3, -0.4));
            m = glm::scale(m, glm::vec3(4.0, 1.5, -1.0));
            Basic.SetModel(m);
            cube.Render();

            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(-2.9, 2.6, -0.7));
            m = glm::scale(m, glm::vec3(1.0, 1.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(1.2, 3, -0.4));
            m = glm::scale(m, glm::vec3(2.0, 1.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(1.9, 3, -0.7));
            m = glm::scale(m, glm::vec3(3.0, 2.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(-8, 3, -0.4));
            m = glm::scale(m, glm::vec3(4.0, 1.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

        }
        
        //Sun
        Basic.SetColor(1, 0.84, 0.43);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, 2, 1));
        m = glm::scale(m, glm::vec3(0.5, 0.5, -1));
        Basic.SetModel(m);
        cube.Render();
        
        glDisable(GL_DEPTH_TEST);
        //Islands
        Basic.SetColor(0.96, 0.69, 0.27);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, -13));
        m = glm::scale(m, glm::vec3(30, 6, -20));
        Basic.SetModel(m);
        cube.Render();

        Basic.SetColor(0.96, 0.69, 0.27);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(-30, -2, -13));
        m = glm::scale(m, glm::vec3(20, 6, -40));
        Basic.SetModel(m);
        cube.Render();

        Basic.SetColor(0.96, 0.69, 0.27);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(40, -2, 2));
        m = glm::scale(m, glm::vec3(10, 4, -10));
        Basic.SetModel(m);
        cube.Render();

        Basic.SetColor(0.96, 0.69, 0.27);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(30, -2, -40));
        m = glm::scale(m, glm::vec3(25, 4, -10));
        Basic.SetModel(m);
        cube.Render();

        glEnable(GL_DEPTH_TEST);
        //Palm tree
        Basic.SetColor(0.37, 0.19, 0.11);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(1, 1, -13));
        m = glm::scale(m, glm::vec3(0.6, 10, -1));
        Basic.SetModel(m);
        cube.Render();

        //Palm tree leaves
        Basic.SetColor(0.16, 0.5, 0.18);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(1, 3.9, -13));
        m = glm::rotate(m, glm::radians(0.0f), glm::vec3(1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1, 3, -1));
        Basic.SetModel(m);
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(1.5, 3.9, -13));
        m = glm::rotate(m, glm::radians(110.0f), glm::vec3(1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1, 3, -1));
        Basic.SetModel(m);
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.6, 3.8, -13));
        m = glm::rotate(m, glm::radians(90.0f), glm::vec3(-1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1, 3, -1));
        Basic.SetModel(m);
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.4, 3.5, -13));
        m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0, 1.0, 1.0));
        m = glm::scale(m, glm::vec3(1, 4, -1));
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(1.4, 3.5, -13));
        m = glm::rotate(m, glm::radians(135.0f), glm::vec3(1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1, 4, -1));
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(1.4, 3.2, -13));
        m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.6, 4, -1));
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 3.2, -13));
        m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0, -1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.6, 4, -1));
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.6, 3.0, -13));
        m = glm::rotate(m, glm::radians(250.0f), glm::vec3(-1.0, -1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.6, 3, -1));
        cube.Render();

        m = glm::translate(glm::mat4(1.0f), glm::vec3(1.4, 3.0, -13));
        m = glm::rotate(m, glm::radians(90.0f), glm::vec3(-1.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.6, 3, -1));
        Basic.SetModel(m);
        cube.Render();

        //Doggo
        m = glm::scale(glm::mat4(1.0f) , glm::vec3(0.2, 0.2, 0.2));
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.6, 0.0, -13));
        Basic.SetModel(m);
        Doggo.Render();

        glUseProgram(0);
        glfwSwapBuffers(Window);

        FrameEndTime = glfwGetTime();
        dt = FrameEndTime - FrameStartTime;
        if (dt < TargetFPS) {
            int DeltaMS = (int)((TargetFrameTime - dt) * 1e3f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            FrameEndTime = glfwGetTime();
        }
        dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
        cloudsEnabled = !cloudsEnabled;
}

void toggle_clouds() {
    cloudsEnabled = !cloudsEnabled;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
*/