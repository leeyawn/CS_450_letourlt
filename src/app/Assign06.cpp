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
glm::vec3 eye = glm::vec3(0, 0, 1);
glm::vec3 lookAt = glm::vec3(0, 0, 0);
glm::vec2 mousePos;

struct PointLight {
    glm::vec4 pos = glm::vec4(0,0,0,1);
    glm::vec4 color = glm::vec4(1,1,1,1);
};

PointLight light;

glm::mat4 makeLocalRotate(glm::vec3 offset, glm::vec3 axis, float angle) {
	glm::mat4 translateNegativeOffset = glm::translate(glm::mat4(1.0f), -offset);
	glm::mat4 rotateAngle = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
	glm::mat4 translateOffset = glm::translate(glm::mat4(1.0f), offset);

	glm::mat4 compTransformation = translateOffset * rotateAngle * translateNegativeOffset;

	return compTransformation; 
}

glm::mat4 makeRotateZ(glm::vec3 offset) {
    float radians = glm::radians(rotAngle);

    glm::mat4 translateNegativeOffset = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translateOffset = glm::translate(glm::mat4(1.0f), offset);

    return translateOffset * rotateZ * translateNegativeOffset;
}

void renderScene(vector<MeshGL> &allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level, GLint normMatLoc, glm::mat4 viewMat) {
    glm::mat4 nodeT;
	aiMatToGLM4(node->mTransformation, nodeT);
    glm::mat4 modelMat = parentMat * nodeT;
    glm::vec3 pos = glm::vec3(modelMat[3]);
    glm::mat4 R = makeRotateZ(pos);
    glm::mat4 tmpModel = R * modelMat;

    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(viewMat * modelMat)));

    glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));
    glUniformMatrix3fv(normMatLoc, 1, false, glm::value_ptr(normalMat));


    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        int index = node->mMeshes[i];
        drawMesh(allMeshes.at(index));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1, normMatLoc, viewMat);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glm::vec3 cameraDir = glm::normalize(lookAt - eye);

    glm::vec3 xAxis = glm::normalize(glm::cross(cameraDir, glm::vec3(0, 1, 0)));

    float speed = 0.1f;

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
		else if(key == GLFW_KEY_W) {
			eye += cameraDir * speed;
            lookAt += cameraDir * speed;
		}
		else if(key == GLFW_KEY_S) {
			eye -= cameraDir * speed;
            lookAt -= cameraDir * speed;
		}
		else if(key == GLFW_KEY_D) {
			eye += xAxis * speed;
			lookAt += xAxis * speed;
		}
		else if(key == GLFW_KEY_A) {
			eye -= xAxis * speed;
			lookAt -= xAxis * speed;
		}
		else if(key == GLFW_KEY_1) {
			light.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else if(key == GLFW_KEY_2) {
			light.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if(key == GLFW_KEY_3) {
			light.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if(key == GLFW_KEY_4) {
			light.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
	}
}

static void mouse_motion_callback(GLFWwindow *window, double xpos, double ypos) {
	glm::vec2 currentMousePos = glm::vec2(xpos, ypos);
    glm::vec2 relMouse = currentMousePos - mousePos;

	int fw, fh;
    glfwGetFramebufferSize(window, &fw, &fh);

    if(fw > 0 && fh > 0) {
        relMouse.x /= fw;
        relMouse.y /= fh;

        relMouse = -relMouse;

		// relative x motion 
		glm::vec4 lookAtV = glm::vec4(lookAt, 1.0);
		glm::mat4 rotateGlobalY = makeLocalRotate(eye, glm::vec3(0, 1, 0), 30.0f * relMouse.x);
		lookAtV = rotateGlobalY * lookAtV;
		lookAt = glm::vec3(lookAtV);

		// calculate local X axis
        glm::vec3 xAxis = glm::normalize(glm::cross(lookAt - eye, glm::vec3(0, 1, 0)));

        // relative y motion
        glm::mat4 rotateLocalX = makeLocalRotate(eye, xAxis, 30.0f * relMouse.y);
        lookAtV = glm::vec4(lookAt, 1.0);
        lookAtV = rotateLocalX * lookAtV;
        lookAt = glm::vec3(lookAtV);
    }

	mousePos = glm::vec2(xpos, ypos);
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	m.vertices.clear();
	m.indices.clear();

	for(unsigned int i = 0; i < mesh -> mNumVertices; i++) {
		Vertex vertex;
		vertex.position = glm::vec3(mesh -> mVertices[i].x, mesh -> mVertices[i].y, mesh -> mVertices[i].z);
		vertex.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		vertex.normal = glm::normalize(glm::vec3(mesh -> mNormals[i].x, mesh -> mNormals[i].y, mesh -> mNormals[i].z));
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
	GLFWwindow* window = setupGLFW("assign06: letourlt", 4, 1, 800, 800, DEBUG_MODE);

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
		string vertexCode = readFileToString("./shaders/assign06/Basic.vs");
		string fragCode = readFileToString("./shaders/assign06/Basic.fs");

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

	// assign05 stuff
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	mousePos = glm::vec2(mx, my);
	glfwSetCursorPosCallback(window, mouse_motion_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	GLint viewMatLoc = glGetUniformLocation(programID, "viewMat");
	GLint projMatLoc = glGetUniformLocation(programID, "projMat");
	GLint normMatLoc = glGetUniformLocation(programID, "normMat");
	GLint lightPosLoc = glGetUniformLocation(programID, "light.pos");
    GLint lightColorLoc = glGetUniformLocation(programID, "light.color");


	light.pos = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    light.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//
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

		// camera
        glm::mat4 viewMat = glm::lookAt(eye, lookAt, glm::vec3(0, 1, 0));
        glUniformMatrix4fv(viewMatLoc, 1, false, glm::value_ptr(viewMat));

		// aspect ratio 
		float aspectRatio = (fwidth > 0 && fheight > 0) ? static_cast<float>(fwidth) / fheight : 1.0f;

		// projection
		glm::mat4 projMat = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 50.0f);
        glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));

		// light
        glm::vec4 lightPos = viewMat * light.pos;
        glUniform4fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform4fv(lightColorLoc, 1, glm::value_ptr(light.color));

/*
		// Draw object
		//drawMesh(mgl);
		for(MeshGL& meshGL : meshGLs) {
			drawMesh(meshGL);
		}	
*/
		// Draw object
        renderScene(meshGLs, scene->mRootNode, glm::mat4(1.0), modelMatLoc, 0, normMatLoc, viewMat);

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
