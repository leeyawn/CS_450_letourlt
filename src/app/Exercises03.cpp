#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Shader.hpp>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

using namespace std;

static void error_callback(int error, const char* desc) {
    cerr << "ERROR " << error << ": " << desc << endl;
}

int main(int argc, char **argv) {
    cout << "BEGIN openGL ADVENTURE!" << endl; 

    glm::vec3 A(1,4,0);
    glm::vec3 B = glm::vec3(2,3,2);
    glm::vec3 C = B - A;

    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        cerr << "ERROR: GLFW FAILLED" << endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    int frameWidth = 800;
    int frameHeight = 600;
    GLFWwindow *window = glfwCreateWindow(frameWidth, frameHeight, "Exercises03", NULL, NULL);

    if(!window) {
        cerr << "ERROR: WINDOW FAILED" << endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = true;
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        cerr << "GLEW ERROR: ";
        cerr << glewGetErrorString(err) << endl;
        glfwTerminate();
        exit(1);
    }

    string vertCode = readFileToString("./shaders/Exercises03/Simple.vs");
    string fragCode = readFileToString("./shaders/Exercises03/Simple.fs");

    GLuint vertID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vertPtr = vertCode.c_str();
    const char *fragPtr = fragCode.c_str();
    glShaderSource(vertID, 1, &vertPtr, NULL);
    glShaderSource(fragID, 1, &fragPtr, NULL);

    glCompileShader(vertID);
    glCompileShader(fragID);

    GLuint progID = glCreateProgram();
    glAttachShader(progID, vertID);
    glAttachShader(progID, fragID);
    glLinkProgram(progID);

    glDeleteShader(vertID);
    glDeleteShader(fragID);


    vector<GLfloat> vertOnly = {
        -0.3f, -0.3f, 0.0f,
        0.3f, -0.3f, 0.0f,
        -0.3f, 0.3f, 0.0f,
        0.3f, 0.3f, 0.0f,
    };

    vector<GLuint> indices = { 0, 1, 2, 1, 3, 2 };

    GLuint VBO = 0;
    GLuint EBO = 0;
    GLuint VAO = 0;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertOnly.size()*sizeof(float), 
                            vertOnly.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 3*sizeof(float), 0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), 
                            indices.data(), GL_STATIC_DRAW);

    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glClearColor(0.7, 0.7, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
        glViewport(0, 0, frameWidth, frameHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progID);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        this_thread::sleep_for(chrono::milliseconds(15));
    }

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);

    glUseProgram(0);
    glDeleteProgram(progID);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

