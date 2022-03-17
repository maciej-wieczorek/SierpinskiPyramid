#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include <helpers/RootDir.h>
#include "Shader.h"
#include "Camera.h"

void generatePyramid(glm::vec3 translation, int size, std::vector<glm::mat4>& models);
void updateModels(int depth, std::vector<glm::mat4>& models);
void generateVertices(std::vector<glm::mat4>& models, float* vertices, unsigned int* indices);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
glm::mat4 myPerspective(float horizontalFOV, float aspect, float near, float far);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int depth = 9; // depth for pyramid

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sierpinski pyramid", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // generate pyramid
    std::vector<glm::mat4>models{};
    float* vertices = new float[20 * pow(5, depth)];
    unsigned int* indices = new unsigned int[20 * pow(5, depth)];
    unsigned int numOfElements = pow(5, depth) * 18;
    unsigned int sizeOfVertices = pow(5, depth) * 20 * sizeof(float);
    unsigned int sizeOfIndices = numOfElements * sizeof(unsigned int);
    updateModels(depth, models);
    generateVertices(models, vertices, indices);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    glfwSetKeyCallback(window, key_callback);
    Shader shader(ROOT_DIR "res/shaders/shader.vert", ROOT_DIR "res/shaders/shader.frag");

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfIndices, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);


    shader.use();

    glm::mat4 view{ 1.0f };
    glm::mat4 projection{ 1.0f };

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //glEnable(GL_DEPTH_TEST);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; 
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glBindVertexArray(VAO);

        //projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        projection = myPerspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        shader.setMat4("projection", projection);

        view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        glDrawElements(GL_TRIANGLES, numOfElements, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void generatePyramid(glm::vec3 translation, int size, std::vector<glm::mat4>& models)
{
    if (size == 1)
    {
        glm::mat4 model{ 1.0f };
        model = glm::translate(model, translation);
        models.push_back(model);
    }
    else
    {
        generatePyramid(glm::vec3(-size / 4.f, (static_cast<float>(-size) * sqrtf(2.f)) / 8.f, -size / 4.f) + translation, size / 2, models);
        generatePyramid(glm::vec3(size / 4.f, (static_cast<float>(-size) * sqrtf(2.f)) / 8.f, -size / 4.f) + translation, size / 2, models);
        generatePyramid(glm::vec3(-size / 4.f, (static_cast<float>(-size) * sqrtf(2.f)) / 8.f, size / 4.f) + translation, size / 2, models);
        generatePyramid(glm::vec3(size / 4.f, (static_cast<float>(-size) * sqrtf(2.f)) / 8.f, size / 4.f) + translation, size / 2, models);
        generatePyramid(glm::vec3(0, (static_cast<float>(size) * sqrtf(2.f)) / 8.f, 0) + translation, size / 2, models);
    }
}

void updateModels(int depth, std::vector<glm::mat4>& models)
{
    generatePyramid(glm::vec3(0, 0, 0), pow(2, depth), models);
}

void generateVertices(std::vector<glm::mat4>& models, float* vertices, unsigned int* indices)
{
    int baseVerticesCount = 20;
    glm::vec4 baseVertices[] = {
        glm::vec4(-0.5f, 0, -0.5f, 1.f),
        glm::vec4(0.5f, 0, -0.5f, 1.f),
        glm::vec4(0.5f, 0, 0.5f, 1.f),
        glm::vec4(-0.5f, 0, 0.5f, 1.f),
        glm::vec4(0, sqrtf(2.f) / 2.f, 0, 1.f)
    };

    int baseIndicesCount = 18;
    unsigned int baseIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        0, 4, 3
    };
    float* curVert;
    unsigned int* curIndex;

    for (int i = 0; i < models.size(); i++)
    {
        for (int j = 0; j < 5; j++)
        {
            curVert = &vertices[(baseVerticesCount * i) + j*4];
            glm::vec4 newVec = models[i] * baseVertices[j];
            curVert[0] = newVec.x;
            curVert[1] = newVec.y;
            curVert[2] = newVec.z;
            curVert[3] = newVec.w;
        }

        curIndex = &indices[i * baseIndicesCount];
        for (int j = 0; j < baseIndicesCount; j++)
        {
            curIndex[j] = baseIndices[j] + (i * 5);
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT_SHIFT) camera.MovementSpeed = 100.f;
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) camera.MovementSpeed = 10.f;
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);



    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UPWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWNWARD, deltaTime);

    //if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    //    depth++;
    //if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && depth > 0)
    //    depth--;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

glm::mat4 myPerspective(float horizontalFOV, float aspect, float near, float far)
{
    float a = 1.f / aspect;
    float e = 1 / tanf(0.00872664625 * horizontalFOV); // horizontal (PI/180 * FOV/2)
    // float e = a / tanf(3.14f / 8.f); // vertical
    float l = -1 * (near / e); // left
    float r = 1 * (near / e); // right
    float t = a * (near / e); // top
    float b = -a * (near / e); // bottom

    return glm::mat4
    {
        glm::vec4{(2 * near) / (r - l), 0, 0, 0},
        glm::vec4{0, (2 * near) / (t - b), 0, 0},
        glm::vec4{(r + l) / (r - l), (t + b) / (t - b), -(far + near) / (far - near), -1},
        glm::vec4{0, 0, -(2 * near * far) / (far - near), 0}
    };
}