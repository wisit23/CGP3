#undef GLFW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <iostream>
#include <vector>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory =
    std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);

GLuint LoadTexture(const std::filesystem::path& texturePath)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(
        texturePath.u8string().c_str(),
        &width,
        &height,
        &channels,
        0
    );

    const bool loadedFromFile = data != nullptr;
    unsigned char fallbackPixel[] = {255, 255, 255, 255};

    if (!loadedFromFile)
    {
        std::cerr << "Failed to load texture " << texturePath << ": "
                  << stbi_failure_reason()
                  << ". Using a white fallback texture.\n";
        data = fallbackPixel;
        width = 1;
        height = 1;
        channels = 4;
    }

    GLenum format = GL_RGB;
    switch (channels)
    {
        case 4: format = GL_RGBA; break;
        case 3: format = GL_RGB; break;
        case 2: format = GL_RG; break;
        case 1: format = GL_RED; break;
        default:
            std::cerr << "Unsupported texture format.\n";
            if (loadedFromFile)
                stbi_image_free(data);
            return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (loadedFromFile)
        stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void CreateTriangle()
{
    GLfloat vertices[] =
    {
        // x, y, z, u, v
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  1.0f, 0.0f, 0.5f, 1.0f
    };

    unsigned int indices[] =
    {
        0, 1, 2
    };

    Mesh* triangle = new Mesh();
    triangle->CreateMesh(vertices, indices, 15, 3);
    meshList.push_back(triangle);
}

void CreateShaders()
{
    Shader* shader = new Shader();
    shader->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );
    shaderList.push_back(shader);
}

void Cleanup()
{
    for (Mesh* mesh : meshList)
        delete mesh;
    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;
    shaderList.clear();
}

int main()
{
    Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    CreateTriangle();
    CreateShaders();

    const GLuint texture = LoadTexture(textureDirectory / "container.jpg");
    if (texture == 0)
    {
        Cleanup();
        return 1;
    }

    while (!mainWindow.getShouldClose())
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0]->UseShader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(shaderList[0]->GetUniformLocation("texture_data"), 0);

        meshList[0]->RenderMesh();

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    glDeleteTextures(1, &texture);
    Cleanup();
    return 0;
}
