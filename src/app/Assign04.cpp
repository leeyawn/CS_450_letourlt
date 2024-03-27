#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <GL/glew.h>					
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "MeshData.hpp"
#include "MeshGLData.hpp"
#include "GLSetup.hpp"
#include "Shader.hpp"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utility.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

float rotAngle = 0.0f;

glm::mat4 makeRotateZ(glm::vec3 offset) {
    float radians = glm::radians(rotAngle);

    glm::mat4 translateNegativeOffset = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translateOffset = glm::translate(glm::mat4(1.0f), offset);

    return translateOffset * rotateZ * translateNegativeOffset;
}

void renderScene(vector<MeshGL> &allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level) {
    glm::mat4 nodeT;

	aiMatToGLM4(node->mTransformation, nodeT);

    glm::mat4 modelMat = parentMat * nodeT;

    glm::vec3 pos = glm::vec3(modelMat[3]);

    glm::mat4 R = makeRotateZ(pos);

    glm::mat4 tmpModel = R * modelMat;

    glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        int index = node->mMeshes[i];
        drawMesh(allMeshes.at(index));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        if(key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
        else if(key == GLFW_KEY_J) {
            rotAngle += 1.0;
        }
		else if(key == GLFW_KEY_K) {
            rotAngle -= 1.0;
        }
	}
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	m.vertices.clear();
	m.indices.clear();

	for(unsigned int i = 0; i < mesh -> mNumVertices; i++) {
		Vertex vertex;
		vertex.position = glm::vec3(mesh -> mVertices[i].x, mesh -> mVertices[i].y, mesh -> mVertices[i].z);
		vertex.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		m.vertices.push_back(vertex);
	}

	for(unsigned int i = 0; i < mesh -> mNumFaces; i++) {
		aiFace face = mesh -> mFaces[i];

		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			m.indices.push_back(face.mIndices[j]);
		}
	}
}

// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimpleQuad(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);
}

// Create very simple mesh: a pentagon (5 vertices, 9 indices, 3 triangles)
void createSimplePentagon(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex upperMiddle;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.7, 0.3, 0.0);
	upperRight.position = glm::vec3(0.7, 0.3, 0.0);
	lowerLeft.position = glm::vec3(-0.45, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.45, -0.5, 0.0);
	upperMiddle.position = glm::vec3(0, 0.7, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	upperMiddle.color = glm::vec4(0.0, 0.0, 0.0, 0.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	m.vertices.push_back(upperMiddle);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);

	m.indices.push_back(4);
	m.indices.push_back(0);
	m.indices.push_back(1);
}

// Main 
int main(int argc, char **argv) {
	// check command argument
	string modelPath = "sampleModels/sphere.obj";
	if(argc >= 2) {
		modelPath = string(argv[1]);
	}

	// assimp import stuff
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | 
											aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

	// import error
	if(!scene || scene -> mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene -> mRootNode) {
		cerr << "Error loading model: " << importer.GetErrorString() << endl;
	}
	
	// Are we in debugging mode?
	bool DEBUG_MODE = true;

	// GLFW setup
	// Switch to 4.1 if necessary for macOS
	GLFWwindow* window = setupGLFW("Assign04: letourlt", 4, 1, 800, 800, DEBUG_MODE);

	// GLEW setup
	setupGLEW(window);

	// Check OpenGL version
	checkOpenGLVersion();

	// Set up debugging (if requested)
	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	// Set the background color to a shade of blue
	glClearColor(0.0f, 0.7f, 0.2f, 1.0f);

	// Create and load shaders
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign04/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign04/Basic.fs");

		// Print out shader code, just to check
		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		// Create shader program from code
		programID = initShaderProgramFromSource(vertexCode, fragCode);
	}
	catch (exception e) {		
		// Close program
		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}

	// Create simple pentagon
	//Mesh m;
	//createSimplePentagon(m);

	// Create OpenGL mesh (VAO) from data
	//MeshGL mgl;
	//createMeshGL(m, mgl);

	vector<MeshGL> meshGLs;

	for(unsigned int i = 0; i < scene -> mNumMeshes; i++) {
		Mesh mesh;
		MeshGL meshGL;

		extractMeshData(scene -> mMeshes[i], mesh);

		createMeshGL(mesh, meshGL);

		meshGLs.push_back(meshGL);
	}
	
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);


	glfwSetKeyCallback(window, keyCallback);

    GLint modelMatLoc = glGetUniformLocation(programID, "modelMat");


	//
	//
	//
	while (!glfwWindowShouldClose(window)) {
		// Set viewport size
		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		// Clear the framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program
		glUseProgram(programID);

/*
		// Draw object
		//drawMesh(mgl);
		for(MeshGL& meshGL : meshGLs) {
			drawMesh(meshGL);
		}	
*/
		// Draw object
		renderScene(meshGLs, scene->mRootNode, glm::mat4(1.0), modelMatLoc, 0);

		// Swap buffers and poll for window events		
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Sleep for 15 ms
		this_thread::sleep_for(chrono::milliseconds(15));
	}

	// Clean up mesh
	//cleanupMesh(mgl);
	for(MeshGL& meshGL : meshGLs) {
		cleanupMesh(meshGL);
	}

	meshGLs.clear();

	// Clean up shader programs
	glUseProgram(0);
	glDeleteProgram(programID);
		
	// Destroy window and stop GLFW
	cleanupGLFW(window);

	return 0;
}
