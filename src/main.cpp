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

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);

void CreateTriangle()
{
    GLfloat vertices[] =
    {
        //x y z
        -1.0f, -1.0f, 0.0f, //index 0
        0.0f, -1.0f,1.0f, //index 1
        1.0f, -1.0f, 0.0f, //index 2
        0.0f,  1.0f, 0.0f //index 3
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
    meshList.push_back(obj1);
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
    for (Mesh* mesh : meshList)
        delete mesh;
    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;
    shaderList.clear();
}

int main()
{
    // OpenGL3.3 
    Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    CreateTriangle();
    CreateShaders();

    //uint = unsigned int
    GLuint uniformModel = 0,uniformView = 0,uniformProjection = 0;
    //perspective projection matrix
    // glm::mat4 projection =glm::perspective(
    //     glm::radians(45.0f),
    //     (GLfloat) mainWindow.getBufferWidth() / (GLfloat) mainWindow.getBufferHeight(),
    //     0.1f,
    //     100.0f
    // );

    //orthographic projection matrix
    glm::mat4 projection = glm::ortho(
        -4.0f, 4.0f,
        -3.0f, 3.0f,
        0.1f, 100.0f
    );

    //paintComponent() 
    while (!mainWindow.getShouldClose())
    {
        // poll inputs 
        glfwPollEvents();

        // clear the screen(r g b a)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 pryamidPosition[]={
            glm::vec3(-0.5f, 0.0f, -2.5f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
        };
        //draw
    for(int i = 0;i<10;i++){
        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetUniformLocation("model");
        uniformView = shaderList[0]->GetUniformLocation("view");
        uniformProjection = shaderList[0]->GetUniformLocation("projection");

        glm::mat4 model (1.0f); //identity matrix
        glm::mat4 view (1.0f); //identity matrix
        // glm::mat4 projection (1.0f); //identity matrix

        model = glm::translate(model, pryamidPosition[i]); //ขยับ model translate the model matrix by 0.3 in x direction
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f)); //scale the model matrix by 0.4 in x and y direction
        
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the model matrix by 30 degrees around the z axis
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection)); //matrix 4x4 floating values

        meshList[0]->RenderMesh();
    }
        glUseProgram(0);

        //double buffer
        mainWindow.swapBuffers();
    }

    // Delete OpenGL objects before the Window destructor destroys the context.
    Cleanup();
    return 0;
}
