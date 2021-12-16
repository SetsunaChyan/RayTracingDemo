#include <memory>
#include <iostream>
#include <cstring>

// OpenCL
#include <CL/opencl.h>

// OpenGL
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "GraphicManager.h"
#include "util.h"


const int WIN_WIDTH = 600;
const int WIN_HEIGHT = 600;

GLFWwindow* window;
GraphicManager cl;

void initOpenGL() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create window
	window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Ray Tracer Demo", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	// Initiate GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}
}

void initOpenCL() {
	cl.setWidthAndHeight(WIN_WIDTH, WIN_HEIGHT);
	cl.init();
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mainLoop() {
	processInput(window);

	cl.runKernel();
	cl.render(window);

	glfwPollEvents();
}

int main(int argc, char** argv) {
	initOpenGL();

	initOpenCL();

	while (!glfwWindowShouldClose(window))
		mainLoop();

	glfwTerminate();

	return 0;
}