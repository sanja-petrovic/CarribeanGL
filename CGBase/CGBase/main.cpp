#include <GL/glew.h>
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
using namespace std;


float
Clamp(float x, float min, float max) {
    return x < min ? min : x > max ? max : x;
}

int WindowWidth = 1920;
int WindowHeight = 1080;
const float TargetFPS = 60.0f;
const std::string WindowTitle = "CaribbeanGL";
const float SEA_LEVEL_CHANGE = 0.05f;
const float FIRE_INTENSITY_CHANGE = 0.01f;

struct Input {
    bool MoveLeft;
    bool MoveRight;
    bool MoveUp;
    bool MoveDown;
    bool LookLeft;
    bool LookRight;
    bool LookUp;
    bool LookDown;
};

struct EngineState {
    Input* mInput;
    Camera* mCamera;
    unsigned mShadingMode;
    bool mDrawDebugLines;
    float mDT;
};

bool cloudsEnabled = true;
bool fireVisible = true;

static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    EngineState* State = (EngineState*)glfwGetWindowUserPointer(window);
    Input* UserInput = State->mInput;
    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_A: UserInput->MoveLeft = IsDown; break;
    case GLFW_KEY_D: UserInput->MoveRight = IsDown; break;
    case GLFW_KEY_W: UserInput->MoveUp = IsDown; break;
    case GLFW_KEY_S: UserInput->MoveDown = IsDown; break;

    case GLFW_KEY_RIGHT: UserInput->LookLeft = IsDown; break;
    case GLFW_KEY_LEFT: UserInput->LookRight = IsDown; break;
    case GLFW_KEY_UP: UserInput->LookUp = IsDown; break;
    case GLFW_KEY_DOWN: UserInput->LookDown = IsDown; break;

    case GLFW_KEY_C: {
        if (IsDown) {
            cloudsEnabled ^= true; break;
        }
    } break;

    case GLFW_KEY_L: {
        if (IsDown) {
            State->mDrawDebugLines ^= true; break;
        }
    } break;

    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

static void
FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    WindowWidth = width;
    WindowHeight = height;
    glViewport(0, 0, width, height);
}


static void
HandleInput(EngineState* state) {
    Input* UserInput = state->mInput;
    Camera* FPSCamera = state->mCamera;
    if (UserInput->MoveLeft) FPSCamera->Move(-1.0f, 0.0f, state->mDT);
    if (UserInput->MoveRight) FPSCamera->Move(1.0f, 0.0f, state->mDT);
    if (UserInput->MoveDown) FPSCamera->Move(0.0f, -1.0f, state->mDT);
    if (UserInput->MoveUp) FPSCamera->Move(0.0f, 1.0f, state->mDT);

    if (UserInput->LookLeft) FPSCamera->Rotate(1.0f, 0.0f, state->mDT);
    if (UserInput->LookRight) FPSCamera->Rotate(-1.0f, 0.0f, state->mDT);
    if (UserInput->LookDown) FPSCamera->Rotate(0.0f, -1.0f, state->mDT);
    if (UserInput->LookUp) FPSCamera->Rotate(0.0f, 1.0f, state->mDT);
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

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    EngineState State = { 0 };
    Camera FPSCamera;
    Input UserInput = { 0 };
    State.mCamera = &FPSCamera;
    State.mInput = &UserInput;
    glfwSetWindowUserPointer(Window, &State);

    glfwSetErrorCallback(ErrorCallback);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
    glfwSetKeyCallback(Window, KeyCallback);

    glViewport(0.0f, 0.0f, WindowWidth, WindowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    unsigned WaterDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/background-sea-water.jpg");
    unsigned WaterSpecularTexture = Texture::LoadImageToTexture("ki61/textures/water-specular.jpg");
    unsigned SandDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/sand.jpg");
    unsigned LeafDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/leaf.jpg");
    unsigned TreeDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/tree.jpg");
    unsigned CloudDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/cloud.png");
    unsigned CloudSpecularTexture = Texture::LoadImageToTexture("ki61/textures/cloud-specular.png");
    unsigned FireDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/fire.jpg");
    unsigned LighthouseDiffuseTexture = Texture::LoadImageToTexture("ki61/textures/lighthouse.png");


    std::vector<float> CubeVertices = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        // RIGHT SIDE
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        // BOTTOM SIDE
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
    };

    unsigned CubeVAO;
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);
    unsigned CubeVBO;
    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, CubeVertices.size() * sizeof(float), CubeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    Shader ColorShader("shaders/color.vert", "shaders/color.frag");

    Shader PhongShaderMaterialTexture("shaders/basic.vert", "shaders/phong_material_texture.frag");
    glUseProgram(PhongShaderMaterialTexture.GetId());
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Direction", glm::vec3(1.0f, -15.0f, -15.0f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(0.5, 0.47, 0.32)); //žućkasta ambijentalna
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(0.5, 0.47, 0.32)); //žućkasta difuzna
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(0.9f, 0.9f, 0.9f)); //bela spekularna 
    PhongShaderMaterialTexture.SetUniform1f("uDirLight.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uDirLight.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uDirLight.Kq", 0.092f);

    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Position", glm::vec3(-5.0f, -12.0f, -27.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ka", glm::vec3(1.0f, 0.58f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Kd", glm::vec3(1.0f, 0.58f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ks", glm::vec3(1.0f, 0.58f, 0.0f));
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kc", 0.05f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kq", 0.032f);

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Position", glm::vec3(40, -10, -70));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Direction", glm::vec3(-170, 0, 1000));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ka", glm::vec3(1.0f, 1.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Kd", glm::vec3(1.0f, 1.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ks", glm::vec3(1.0f, 1.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kc", 0.05f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kl", 0.1f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kq", 0.02f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.InnerCutOff", glm::cos(glm::radians(30.0f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.OuterCutOff", glm::cos(glm::radians(205.0f)));

    
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
    PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 128.0f);
    glUseProgram(0);

    glm::mat4 Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
    glm::mat4 ModelMatrix(1.0f);

    float TargetFrameTime = 1.0f / TargetFPS;
    float StartTime = glfwGetTime();
    float EndTime = glfwGetTime();
    glClearColor(0.3, 0.66, 0.58, 1.0);

    Shader* CurrentShader = &PhongShaderMaterialTexture;
    float seaLevel = 12.0f;
    float seaLevelChange = SEA_LEVEL_CHANGE;
    float fireLightIntensity = 0.05;
    float fireIntensityChange = FIRE_INTENSITY_CHANGE;
    Model Cat("ki61/12221_Cat_v1_l3.obj");
    if (!Cat.Load())
    {
        std::cout << "Failed to load model!\n";
        glfwTerminate();
        return -1;
    }
    float gComponent = 0.58;
    float bComponent = 0;
    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();
        HandleInput(&State);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
        StartTime = glfwGetTime();
        glUseProgram(CurrentShader->GetId());
        CurrentShader->SetProjection(Projection);
        CurrentShader->SetView(View);
        CurrentShader->SetUniform3f("uViewPos", FPSCamera.GetPosition());

        #pragma region Lighthouse
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(40, -14, -70));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 10, -1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        #pragma endregion

        #pragma region Sea
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, -23, -13));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(700, seaLevel, 400));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, WaterDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, WaterSpecularTexture);
        glBindVertexArray(CubeVAO);
        seaLevel += seaLevelChange;
        if (seaLevel > 15) seaLevelChange = -SEA_LEVEL_CHANGE;
        if (seaLevel < 12) seaLevelChange = SEA_LEVEL_CHANGE;
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        #pragma region Islands
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.6, -17.5, -30));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(40, 6, 10));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(60, -17.5, -50));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10, 6, 10));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-70, -17.5, -70));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30, 6, 10));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        #pragma region Clouds
        if (cloudsEnabled) {
            ModelMatrix = glm::mat4(1.0f);
            ModelMatrix = glm::translate(ModelMatrix, glm::vec3(30, 17, -70));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30, 10, 10));
            CurrentShader->SetModel(ModelMatrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CloudSpecularTexture);
            glBindVertexArray(CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

            ModelMatrix = glm::mat4(1.0f);
            ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-30, 17, -70));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(20, 8, 10));
            CurrentShader->SetModel(ModelMatrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CloudSpecularTexture);
            glBindVertexArray(CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

            ModelMatrix = glm::mat4(1.0f);
            ModelMatrix = glm::translate(ModelMatrix, glm::vec3(80, 14, -75));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(15, 5, 6));
            CurrentShader->SetModel(ModelMatrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CloudSpecularTexture);
            glBindVertexArray(CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

            ModelMatrix = glm::mat4(1.0f);
            ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-80, 34, -75));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(15, 5, 6));
            CurrentShader->SetModel(ModelMatrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CloudSpecularTexture);
            glBindVertexArray(CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        }
        #pragma endregion

        #pragma region Model
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05, 0.05, 0.05));
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.6, -253.5, -500));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        CurrentShader->SetModel(ModelMatrix);
        Cat.Render(); 
        #pragma endregion

        #pragma region Palm tree
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.5, -6.5, -27.5));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 14, 1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TreeDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        #pragma region Palm leaves
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-0.5, 1, -25.5));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(30.0f), glm::vec3(1.0, 1.0, 0.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5, 1, 1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, LeafDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.5, 2, -27.5));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(120.0f), glm::vec3(-0.8, 0.5, 0.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5, 1, 1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, LeafDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(2.5, 2, -27.5));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(75.0f), glm::vec3(0.5, 0.5, 0.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5, 1, 1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, LeafDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(3.5, 1, -25.5));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(330.0f), glm::vec3(1.0, 1.0, 0.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5, 1, 1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, LeafDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        #pragma region Sun
        ModelMatrix = glm::mat4(1.0f); 
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0, 17, -50));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 1, -1));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FireDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        #pragma region Fire
        fireLightIntensity += fireIntensityChange;
        if (fireLightIntensity > 1.0) fireIntensityChange = -FIRE_INTENSITY_CHANGE;
        if (fireLightIntensity < 0.0) fireIntensityChange = FIRE_INTENSITY_CHANGE;
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-7, -12, -27));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3, 3, -4));
        CurrentShader->SetModel(ModelMatrix);
        CurrentShader->SetUniform1f("uPointLight.Kc", fireLightIntensity);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FireDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        #pragma endregion

        
        glBindVertexArray(0);
        glUseProgram(0);
        glfwSwapBuffers(Window);

        EndTime = glfwGetTime();
        float WorkTime = EndTime - StartTime;
        if (WorkTime < TargetFrameTime) {
            int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            EndTime = glfwGetTime();
        }
        State.mDT = EndTime - StartTime;
    }

    glfwTerminate();
    return 0;
}