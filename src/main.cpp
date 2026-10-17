#undef GLFW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <vector>
#include <iostream>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

//Global Var
GLuint uniformModel = 0,uniformView = 0,uniformProjection = 0,uniformTextureData = 0,uniformTextureData_2 = 0;
GLuint texture_data ;
glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f); //set the light colour to white
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f); //camera position
glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 0.0f);

float yaw = -90.0f; //camera yaw
float pitch = 0.0f; //camera pitch

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;
Shader* depthShader;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory =
    std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);
const std::filesystem::path modelDirectory =
    std::filesystem::u8path(OPENGL_STARTER_MODEL_DIR);

void CreateTriangle()
{
    GLfloat vertices[] =
    {
        //x y z u v 
        -1.0f, -1.0f, 0.0f,0.0f,0.0f, //index 0
        0.0f, -1.0f,1.0f,0.5f,0.0f, //index 1
        1.0f, -1.0f, 0.0f,1.0f,0.0f, //index 2
        0.0f,  1.0f, 0.0f,0.5f,1.0f  //index 3
    };

    unsigned int indices[] =
    {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 20, 12);
    meshList.push_back(obj1);
}

void CreateOBJ(const std::filesystem::path& modelPath)
{
    Mesh* obj = new Mesh();
    bool loaded = obj->CreateMeshFromOBJ(modelPath.u8string().c_str());

    if (!loaded)
    {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return;
    }

    meshList.push_back(obj);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );
    shaderList.push_back(shader1);

        Shader* shader2 = new Shader();
    shader2->CreateFromFiles(
        shaderDirectory / "lightShader.vert",
        shaderDirectory / "lightShader.frag"
    );
    shaderList.push_back(shader2);

    Shader* shader3 = new Shader();
    shader3->CreateFromFiles(
        shaderDirectory / "bgShader.vert",
        shaderDirectory / "bgShader.frag"
    );
    shaderList.push_back(shader3);

    depthShader = new Shader();
    depthShader->CreateFromFiles(
        shaderDirectory / "depthShader.vert",
        shaderDirectory / "depthShader.frag"
    );
}

GLuint LoadTexture(const std::filesystem::path& texturePath)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char * data = stbi_load(texturePath.u8string().c_str(), 
                &width, &height, &channels, 0);

    if (data == nullptr)
    {
        std::cerr << "Failed to load texture: "<<stbi_failure_reason() << "\n";
        return 0;
    }

    GLenum format = GL_RGB; // Default format
    switch (channels)
    {
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
            std::cerr << "Unsupported number of channels: " << channels << "\n";
            stbi_image_free(data);
            return 0;

    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //wrap s = ในเเกน s ถ้ากระดาษหมดให้ repeat
    //wrap t = ในเเกน t ถ้ากระดาษหมดให้ repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
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

void UpdateMouseLook(Window &window){
    double mouseX, mouseY;
    glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);
    //pull first mouseX
    static double lastX = mouseX;
    static double lastY = mouseY;
    
    float xOffset = static_cast<float>(mouseX - lastX);
    float yOffset = static_cast<float>(mouseY - lastY);
    float sensitivity = 0.1f;
    //update lastX and lastY
    lastX=mouseX;
    lastY=mouseY;

    yaw += xOffset * sensitivity; //update yaw
    pitch -= yOffset * sensitivity; //update pitch +เงย -ก้ม

    if(pitch > 89.0f) //limit pitch
        pitch = 89.0f;
    if(pitch < -89.0f) //limit pitch
        pitch = -89.0f;
}

void RenderScene(glm::mat4 view, glm::mat4 projection)
{
    //render obj
    glm::vec3 pryamidPosition[]={
        glm::vec3(0.0f, 0.0f, -2.5f),
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
        // shaderList[0]->UseShader();
        // uniformModel = shaderList[0]->GetUniformLocation("model");
        // uniformView = shaderList[0]->GetUniformLocation("view");
        // uniformProjection = shaderList[0]->GetUniformLocation("projection");
        // uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");
        // uniformTextureData_2 = shaderList[0]->GetUniformLocation("texture_data_2");
        glUniform1i(uniformTextureData, 0); //set the texture unit to 0
        // glUniform1i(uniformTextureData_2, 1); //set the texture unit to 1

        glActiveTexture(GL_TEXTURE0); //set the active texture unit to 0
        glBindTexture(GL_TEXTURE_2D, texture_data); //bind the texture to the

        // glUniform3fv(shaderList[0]->GetUniformLocation("lightColour"), 
        //             1, 
        //             (GLfloat*)&lightColour); //set the light colour to white
        // glUniform3fv(shaderList[0]->GetUniformLocation("lightPos"), 
        //             1, 
        //             (GLfloat*)&lightPos); //set the light position
        // glUniform3fv(shaderList[0]->GetUniformLocation("viewPos"), 
        //             1, 
        //             (GLfloat*)&cameraPos); //set the view position

        // shaderList[0]->GetUniformLocation("lightColour"); //set the light colour to white
        // glActiveTexture(GL_TEXTURE1); //set the active texture unit to 1
        // glBindTexture(GL_TEXTURE_2D, texture_data_2); //bind the texture to the

        glm::mat4 model (1.0f); //identity matrix
        // glm::mat4 view (1.0f); //identity matrix
        // glm::mat4 projection (1.0f); //identity matrix

        model = glm::translate(model, pryamidPosition[i]); //ขยับ model translate the model matrix by 0.3 in x direction
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f)); //scale the model matrix by 0.4 in x and y direction
        
        // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the model matrix by 30 degrees around the z axis
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection)); //matrix 4x4 floating values

        meshList[0]->RenderMesh();

        glBindTexture(GL_TEXTURE_2D, 0); //bind the texture to the
    }
}

int main()
{
    // OpenGL3.3 
    Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    //Load obj
    CreateOBJ(modelDirectory / "suzanne.obj");
    CreateTriangle();
    CreateShaders();

    //load light
    CreateOBJ(modelDirectory / "cube.obj");

    //load bg
    CreateOBJ(modelDirectory / "cube.obj");

    texture_data = LoadTexture(textureDirectory / "uvmap.png");
    // GLuint texture_data_2 = LoadTexture(textureDirectory / "paper.png");
    if (texture_data == 0)
    {
        glDeleteTextures(1, &texture_data);
        return 1;
    }
    // if (texture_data_2 == 0)
    // {
    //     glDeleteTextures(1, &texture_data_2);
    //     return 1;
    // }
    //uint = unsigned int
    // GLuint uniformModel = 0,uniformView = 0,uniformProjection = 0,uniformTextureData = 0,uniformTextureData_2 = 0;

    //perspective projection matrix
    glm::mat4 projection =glm::perspective(
        glm::radians(45.0f),
        (GLfloat) mainWindow.getBufferWidth() / (GLfloat) mainWindow.getBufferHeight(),
        0.1f,
        100.0f
    );

    //orthographic projection matrix
    // glm::mat4 projection = glm::ortho(
    //     -4.0f, 4.0f,
    //     -3.0f, 3.0f,
    //     0.1f, 100.0f
    // );
    
    // glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 20.0f);

    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f); //camera target
    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos); //camera direction
    
    // float yaw = -90.0f; //camera yaw
    // float pitch = 0.0f; //camera pitch
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); //calculate camera direction x
    cameraDirection.y = sin(glm::radians(pitch)); //calculate camera direction y
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); //calculate camera direction z
    
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); //camera up vector
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection,up)); //camera right vector
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection)); //camera up vector

    float lastTime = static_cast<float>(glfwGetTime());

    //setup shadow depth map
    //Declare FrameBuffer 
    GLuint depthMapFBO; //FBO = Frame Buffer Object
    glGenFramebuffers(1, &depthMapFBO);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    //Declare Depth Map Texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
    GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    

    //Blind FBO and Texture
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //paintComponent() 
    while (!mainWindow.getShouldClose())
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // poll inputs 
        glfwPollEvents();
        UpdateMouseLook(mainWindow);
        //inputs
        float moveSpeed = 5.0f;
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos = cameraPos + cameraDirection * moveSpeed*deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos = cameraPos - cameraDirection * moveSpeed*deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos = cameraPos - cameraRight * moveSpeed*deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos = cameraPos + cameraRight * moveSpeed*deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            yaw = -90.0f; //reset yaw
            pitch = 0.0f; //reset pitch
            cameraPos = glm::vec3(0.0f, 0.0f, 0.0f); //reset camera position
        }

        lightPos.x = 1.0f + sin(glfwGetTime())*2.0f;
        lightPos.y = sin(glfwGetTime())/2.0f;

        // clear the screen(r g b a)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST); //disable culling

        glm::mat4 view (1.0f); //identity matrix
        glm::mat4 cameraRotateMat(1.0f); //identity matrix
        glm::mat4 cameraPosMat(1.0f); //identity matrix

        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); //calculate camera direction x
        cameraDirection.y = sin(glm::radians(pitch)); //calculate camera direction y
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); //calculate camera direction z

        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection,up)); //camera right vector คำนวณใหม่ทุก loop
        glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection)); //camera up vector  คำนวณใหม่ทุก loop

        
        // glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f); //camera position
        // glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f); //camera target
        // glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos); //camera direction
        
        // float yaw = -90.0f; //camera yaw
        // float pitch = 0.0f; //camera pitch

        
        // glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); //camera up vector
        // glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection)); //camera right vector
        // glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection)); //camera up vector
        


        // ใช้ glm::lookAt() เพื่อสร้าง view matrixเเทนได้
        // cameraPosMat[3][0] = -cameraPos.x; //translate the camera position matrix by -cameraPos.x in x direction
        // cameraPosMat[3][1] = -cameraPos.y; //translate the camera position matrix by -cameraPos.y in y direction
        // cameraPosMat[3][2] = -cameraPos.z; //translate the camera position matrix by -cameraPos.z in z direction
        // cameraRotateMat[0] = glm::vec4(cameraRight.x,cameraUp.x,-cameraDirection.x,0.0f); //set the camera right vector
        // cameraRotateMat[1] = glm::vec4(cameraRight.y,cameraUp.y,-cameraDirection.y,0.0f); //set the camera up vector
        // cameraRotateMat[2] = glm::vec4(cameraRight.z,cameraUp.z,-cameraDirection.z,0.0f); //set the camera direction vector
        // view = cameraRotateMat * cameraPosMat;


        
        
        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp); //view matrix
        
        //First render to depth map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT); //ใช้เก็บข้อมูลความลึกของ pixel แต่ละ pixel framebuffer ทำได้

        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, -10.0f), up); //วัดจากเเหล่งกำเนิดแสงไปยังจุดศูนย์กลางของ scene เลยตั้งกล้องที่เเหล่งกำเนิดเเสง
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 20.0f); //ใช้ orthographic projection เพราะเเสงเป็น parallel light
        
        depthShader->UseShader();
        uniformModel = depthShader->GetUniformLocation("model");
        uniformView = depthShader->GetUniformLocation("view");
        uniformProjection = depthShader->GetUniformLocation("projection");

        glCullFace(GL_FRONT); //cull front face to avoid peter panning
        RenderScene(lightView, lightProjection); //render scene from light's perspective
        glCullFace(GL_BACK); //cull back face to avoid peter panning

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //Second render to screen
        glViewport(0, 0, mainWindow.getBufferWidth(), mainWindow.getBufferHeight());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //RenderObj
        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetUniformLocation("model");
        uniformView = shaderList[0]->GetUniformLocation("view");
        uniformProjection = shaderList[0]->GetUniformLocation("projection");
        uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");


        glUniform3fv(shaderList[0]->GetUniformLocation("lightColour"), 
                    1, 
                    (GLfloat*)&lightColour); //set the light colour to white
        glUniform3fv(shaderList[0]->GetUniformLocation("lightPos"), 
                    1, 
                    (GLfloat*)&lightPos); //set the light position
        glUniform3fv(shaderList[0]->GetUniformLocation("viewPos"), 
                    1, 
                    (GLfloat*)&cameraPos); //set the view position

        RenderScene(view, projection); 

        // light
        shaderList[1]->UseShader();
        uniformModel = shaderList[1]->GetUniformLocation("model");
        uniformView = shaderList[1]->GetUniformLocation("view");
        uniformProjection = shaderList[1]->GetUniformLocation("projection");

        glm::mat4 model (1.0f);

        model = glm::translate(model, lightPos); //ขยับ model translate the model matrix by 0.3 in x direction
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f)); //scale the model matrix by 0.4 in x and y direction
        
        // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the model matrix by 30 degrees around the z axis
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection)); //matrix 4x4 floating values

        glUniform3fv(shaderList[1]->GetUniformLocation("lightColour"), 
                    1, 
                    (GLfloat*)&lightColour); //set the light colour to white

        meshList[2]->RenderMesh();

        // bg
        shaderList[2]->UseShader();
        uniformModel = shaderList[2]->GetUniformLocation("model");
        uniformView = shaderList[2]->GetUniformLocation("view");
        uniformProjection = shaderList[2]->GetUniformLocation("projection");

        glUniformMatrix4fv(shaderList[2]->GetUniformLocation("lightView"),1,GL_FALSE,glm::value_ptr(lightView));
        glUniformMatrix4fv(shaderList[2]->GetUniformLocation("lightProjection"),1,GL_FALSE,glm::value_ptr(lightProjection));
        
        
        
        model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -8.0f)); //ขยับ model translate the model matrix by 0.3 in x direction
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 0.f)); //scale the model matrix by 0.4 in x and y direction
        
        // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the model matrix by 30 degrees around the z axis
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view)); //matrix 4x4 floating values
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection)); //matrix 4x4 floating values
        glm::vec3 bgColour = glm::vec3(1.0f, 0.5f, 0.5f); //set the background colour to blue
        glUniform3fv(shaderList[2]->GetUniformLocation("bgColour"), 
                    1, 
                    (GLfloat*)&bgColour); //set the background colour to blue
        glBindTexture(GL_TEXTURE_2D, depthMap);
        meshList[2]->RenderMesh();

        glUseProgram(0);

        //double buffer
        mainWindow.swapBuffers();
    }

    // Delete OpenGL objects before the Window destructor destroys the context.
    Cleanup();
    return 0;
}
