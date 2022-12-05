#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "shader.hpp"
#include "model.hpp"
#include "renderable.hpp"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void process_input(GLFWwindow* window);
void toggle_clouds();

const int WindowWidth = 1920;
const int WindowHeight = 1080;
const std::string WindowTitle = "CaribbeanGL";
const float TargetFPS = 60.0f;
const float TargetFrameTime = 1.0f / TargetFPS;


static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WindowWidth / 2.0f;
float lastY = WindowHeight / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool cloudsEnabled = true;

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
    glfwSetCursorPosCallback(Window, mouse_callback);
    glfwSetScrollCallback(Window, scroll_callback);

    

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader Basic("shaders/basic.vert", "shaders/basic.frag");

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
        0, 2, 3,

        4, 6, 7,
        4, 5, 7,

        3, 6, 2,
        3, 6, 7,

        0, 4, 1,
        1, 4, 5,

        0, 6, 2,
        0, 4, 6,

        1, 3, 7,
        1, 7, 5
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0, 0.3, 1.0, 1.0);


    float FrameStartTime = glfwGetTime();
    float FrameEndTime = glfwGetTime();
    glfwSetKeyCallback(Window, key_callback);
    float dt = FrameEndTime - FrameStartTime;
    while (!glfwWindowShouldClose(Window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(Window);
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        FrameStartTime = glfwGetTime();
        glUseProgram(Basic.GetId());

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WindowWidth / (float)WindowHeight, 0.1f, 500.0f);
        Basic.SetProjection(projection);

        glm::mat4 view = camera.GetViewMatrix();
        Basic.SetView(view);
        //Crtanje oblaka

        if (cloudsEnabled) {
            Basic.SetColor(1, 1, 1);

            m = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2, 1.1, -0.4));
            m = glm::rotate(m, glm::radians(35.0f), glm::vec3(1.0, 1.0, 1.0));
            m = glm::scale(m, glm::vec3(3.0, 2.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

            m = glm::translate(glm::mat4(1.0f), glm::vec3(1.50, 2.4, 2.0));
            m = glm::rotate(m, glm::radians(-20.0f), glm::vec3(1.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(3.0f));
            Basic.SetModel(m);
            cube.Render();

            m = glm::translate(glm::mat4(1.0f), glm::vec3(-1.3, 1.6, -0.4));
            m = glm::rotate(m, glm::radians(30.0f), glm::vec3(1.0, 1.0, 1.0));
            m = glm::scale(m, glm::vec3(2.0, 5.0, -1.0));
            Basic.SetModel(m);
            cube.Render();

            m = glm::translate(glm::mat4(1.0f), glm::vec3(-1.6, 1.5, 1.2));
            m = glm::rotate(m, glm::radians(05.0f), glm::vec3(1.0, 0.5, 1.0));
            m = glm::scale(m, glm::vec3(3.0, 2.0, 1.5));
            Basic.SetModel(m);
            cube.Render();

            m = glm::translate(glm::mat4(1.0f), glm::vec3(1, -2.4, -0.25));
            m = glm::scale(m, glm::vec3(3.0, 2.0, 2.0));
            m = glm::rotate(m, glm::radians(15.0f), glm::vec3(1.0, 1.0, 1.0));
            Basic.SetModel(m);
            cube.Render();

            m = glm::translate(glm::mat4(1.0f), glm::vec3(1.5, 1.5, 0.1));
            m = glm::rotate(m, glm::radians(135.0f), glm::vec3(1.0, 1.0, 1.0));
            m = glm::scale(m, glm::vec3(2.0, 1.0, 1.0));
            Basic.SetModel(m);
            cube.Render();
        }

        //Doggo
        Basic.SetColor(0, 0, 0);
        m = glm::scale(glm::mat4(1.0f) , glm::vec3(0.2, 0.2, 0.2));
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

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    /*if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);*/
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

