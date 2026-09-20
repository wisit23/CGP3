#undef GLFW_DLL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <vector>
#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <iostream>

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

float yaw = -90.0f;
float pitch = 0.0f;

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

const std::filesystem::path shaderDirectory = std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory = std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);

GLuint LoadTexture(const std::filesystem::path& texturePath){
    int width =0;
    int height =0;
    int channels =0;

    unsigned char* data = stbi_load(texturePath.u8string().c_str(), &width, &height, &channels, 0);
    const bool loadedFromFile = data != nullptr;
    unsigned char fallbackPixel[] = {255, 255, 255, 255};

    if (!loadedFromFile)
    {
        std::cerr << "Failed to load texture " << texturePath << ": "
                  << stbi_failure_reason() << ". Using a white fallback texture.\n";
        data = fallbackPixel;
        width = 1;
        height = 1;
        channels = 4;
    }

    GLenum format = GL_RGB;
    switch(channels){
        case 4:
            format = GL_RGBA;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 2:
            format = GL_RG;
            break;
        case 1:
            format = GL_RED;
            break;
        default:
            std::cerr << "Unsupported texture format\n";
            stbi_image_free(data);
            return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glGenerateMipmap(GL_TEXTURE_2D);

    if (loadedFromFile)
        stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return  texture;
}
void CreateTriangle()
{
    GLfloat vertices[] =
    { 
        // x y z u v
        -1.0f, -1.0f, 0.0f,0.0f,0.0f,
         0.0f, -1.0f, 1.0f,0.5f,0.0f,
         1.0f, -1.0f, 0.0f,1.0f,0.0f,
         0.0f,  1.0f, 0.0f,0.5f,1.0f
    };

    unsigned int indices[] =
    {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 12, 12);

    for (int i = 0; i < 10; i++)
    {
        meshList.push_back(obj1);
    }
}

void CreateShaders()
{
    Shader* shader1 = new Shader();

    shader1->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );

    shaderList.push_back(shader1);
}

void Cleanup()
{
    if (!meshList.empty())
        delete meshList[0];

    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;

    shaderList.clear();
}

void UpdateMouseLook(Window& window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

    //pull first mouse x
    static double lastX = mouseX;
    static double lastY = mouseY;

    float xoffset = static_cast<float>(mouseX - lastX);
    float yoffset = static_cast<float>(lastY - mouseY);

    float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    //update
    lastX = mouseX;
    lastY = mouseY;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f)
        pitch = -89.0f;
}

int main()
{
    Window mainWindow(WIDTH, HEIGHT, 3, 3);

    if (mainWindow.initialise() != 0)
        return 1;

    CreateTriangle();
    CreateShaders();
    GLuint texture_data = LoadTexture(textureDirectory / "container.jpg");
    GLuint uniformModel = 0, uniformView = 0, uniformProjection = 0, uniformTextureData = 0;
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f,
        100.0f
    );

    float lastTime = static_cast<float>(glfwGetTime());

    glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraTarget(-1.0f, 0.0f, -1.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);

    // float yaw = -90.0f;
    // float pitch = 0.0f;

    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

    while (!mainWindow.getShouldClose())
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        //poll inputs
        glfwPollEvents();

        UpdateMouseLook(mainWindow);

        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection = glm::normalize(cameraDirection);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture_data);
        shaderList[0]->UseShader();


        uniformModel = shaderList[0]->GetUniformLocation("model");
        uniformView = shaderList[0]->GetUniformLocation("view");
        uniformProjection = shaderList[0]->GetUniformLocation("projection");

        // glDisable(GL_DEPTH_TEST);

        glm::mat4 model(1.0f);
        glm::mat4 view(1.0f);
        glm::mat4 cameraRotateMat(1.0f);
        glm::mat4 cameraPostMat(1.0f);

        // glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
        // glm::vec3 cameraTarget(-1.0f, 0.0f, -1.0f);
        // glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);

        // float yaw = -90.0f;
        // float pitch = 0.0f;

        // cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        // cameraDirection.y = sin(glm::radians(pitch));
        // cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        // glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        glm::vec3 forwardDirection = glm::normalize(
            glm::vec3(cameraDirection.x, 0.0f, cameraDirection.z)
        );

        float moveSpeed = 5.0f;

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos = cameraPos + forwardDirection * moveSpeed * deltaTime;
        }

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos = cameraPos - forwardDirection * moveSpeed * deltaTime;
        }

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos = cameraPos - cameraRight * moveSpeed * deltaTime;
        }

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos = cameraPos + cameraRight * moveSpeed * deltaTime;
        }

        cameraPostMat[3][0] = -cameraPos.x;
        cameraPostMat[3][1] = -cameraPos.y;
        cameraPostMat[3][2] = -cameraPos.z;

        cameraRotateMat[0] = glm::vec4(cameraRight.x, cameraUp.x, -cameraDirection.x, 0.0f);
        cameraRotateMat[1] = glm::vec4(cameraRight.y, cameraUp.y, -cameraDirection.y, 0.0f);
        cameraRotateMat[2] = glm::vec4(cameraRight.z, cameraUp.z, -cameraDirection.z, 0.0f);

        view = cameraRotateMat * cameraPostMat;

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        glm::vec3 pyramidPositions[] =
        {
            glm::vec3( 0.0f,  0.0f,  -2.5f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f,  -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f,  -3.5f),
            glm::vec3(-1.7f,  3.0f,  -7.5f),
            glm::vec3( 1.3f, -2.0f,  -2.5f),
            glm::vec3( 1.5f,  2.0f,  -2.5f),
            glm::vec3( 1.5f,  0.2f,  -1.5f),
            glm::vec3(-1.3f,  1.0f,  -1.5f)
        };

        for (int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);

            model = glm::translate(model, pyramidPositions[i]);
            model = glm::rotate(model, glm::radians(2.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(0.8f, 1.6f, 1.0f));
            uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");

            glUniform1i(uniformTextureData, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_data);

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

            meshList[i]->RenderMesh();
        }

        mainWindow.swapBuffers();
    }

    glDeleteTextures(1, &texture_data);
    Cleanup();

    return 0;
}