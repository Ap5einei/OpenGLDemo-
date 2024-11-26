
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

class SimpleObject {
public:
    std::vector<float> vertices;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;

    SimpleObject(const std::vector<float>& verts, glm::vec2 pos, glm::vec2 vel, glm::vec3 col)
        : vertices(verts), position(pos), velocity(vel), color(col) {}

    void update(float deltaTime) {
        position += velocity * deltaTime;
        if (position.x > 1.0f || position.x < -1.0f) velocity.x = -velocity.x;
        if (position.y > 1.0f || position.y < -1.0f) velocity.y = -velocity.y;
    }
};

class Renderer {
public:
    unsigned int shaderProgram;
    unsigned int VAO, VBO;

    Renderer() {
        // Compile shaders
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // Link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Set up vertex buffer and array object
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    void render(const SimpleObject& object) {
        glUseProgram(shaderProgram);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(object.position, 0.0f));
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        unsigned int colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform3fv(colorLoc, 1, glm::value_ptr(object.color));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, object.vertices.size() * sizeof(float), object.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, object.vertices.size() / 2);
    }
};

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Simple Objects Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    Renderer renderer;

    // Create a triangle
    SimpleObject triangle({
        -0.1f, -0.1f,
         0.1f, -0.1f,
         0.0f,  0.1f
        }, glm::vec2(0.0f), glm::vec2(0.3f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Create a square
    SimpleObject square({
        -0.1f, -0.1f,
         0.1f, -0.1f,
         0.1f,  0.1f,
        -0.1f,  0.1f
        }, glm::vec2(0.5f, -0.5f), glm::vec2(-0.4f, 0.3f), glm::vec3(0.0f, 1.0f, 0.0f));

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        triangle.update(deltaTime);
        square.update(deltaTime);

        renderer.render(triangle);
        renderer.render(square);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}