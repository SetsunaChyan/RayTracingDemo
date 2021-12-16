#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "CLManager.h"

class GraphicManager : public CLManager {
private:
	const std::string kernalName = "kernelMain";
	const GLfloat vertexCoords[12] = { -1.0f, -1.0f, 0.0f,
						   -1.0f,  1.0f, 0.0f,
							1.0f,  1.0f, 0.0f,
							1.0f, -1.0f, 0.0f };
	const GLfloat texCoords[8] = { 0.0f, 1.0f,
							0.0f, 0.0f,
							1.0f, 0.0f,
							1.0f, 1.0f };
	GLuint vao, vbo[2], pbo, texture;

	int winWidth, winHeight;
	clock_t lstTime;
	char titleBuffer[100];
	cl_double3 sum[800 * 800];

	cl_mem sphereBuffer, outBuffer, camBuffer, sumBuffer;
	Camera cam;
	cl_int sphereSize;
	cl_ulong frameCount;
	Sphere sphere[20];

	void initData() {
		cam.pos = cl_double3{ 0.0,0.0,0.0 };
		cam.up = cl_double3{ 0.0,1.0,0.0 };
		cam.lookAt = cl_double3{ 1.0,0.0,0.0 };
		cam.theta = CL_M_PI / 3.0;
		cam.winWidth = winWidth;
		cam.winHeight = winHeight;

		Material diffuseMat, metalMat, dielectricMat, fuzzMetalMat;
		Material lightMat, leftWallMat, rightWallMat, floorMat;
		diffuseMat.color = cl_double3{ 0x99,0x00,0xFF } / 256.0;
		diffuseMat.refraction = 0;
		diffuseMat.reflection = 0;
		diffuseMat.type = 1;

		metalMat.color = cl_double3{ 0xFF,0x66,0x33 } / 256.0;
		metalMat.refraction = 0;
		metalMat.reflection = 0;
		metalMat.type = 2;

		dielectricMat.color = cl_double3{ 0xCC,0xFF,0xFF } / 256.0;
		dielectricMat.refraction = 1.5;
		dielectricMat.reflection = 0;
		dielectricMat.type = 3;

		fuzzMetalMat.color = cl_double3{ 0x99,0xFF,0x33 } / 256.0;
		fuzzMetalMat.refraction = 0;
		fuzzMetalMat.reflection = 0;
		fuzzMetalMat.type = 4;

		lightMat.color = cl_double3{ 0xFF,0xFF,0xFF } / (256.0 / 15.0);
		lightMat.refraction = 0;
		lightMat.reflection = 0;
		lightMat.type = 0;

		leftWallMat.color = cl_double3{ 0x33,0xFF,0x00 } / 256.0;
		leftWallMat.refraction = leftWallMat.reflection = 0;
		leftWallMat.type = 1;
		floorMat = rightWallMat = leftWallMat;
		rightWallMat.color = cl_double3{ 0xFF,0x00,0x33 } / 256.0;
		floorMat.color = cl_double3{ 0xDD,0xDD,0xDD } / 256.0;

		sphereSize = 12;
		const cl_double INF = 1e6;
		// light
		sphere[0].radius = 1000;
		sphere[0].pos = cl_double3{ 800, sphere[0].radius + winHeight - 20, 0 };
		sphere[0].mat = lightMat;

		// mask
		sphere[1].radius = INF / 10;
		sphere[1].pos = cl_double3{ 0, INF / 10 + winHeight, 0 };
		sphere[1].mat = floorMat;

		// left wall 
		sphere[2].radius = INF;
		sphere[2].pos = cl_double3{ 0, 0, -INF - winWidth };
		sphere[2].mat = leftWallMat;

		// right wall
		sphere[3].radius = INF;
		sphere[3].pos = cl_double3{ 0, 0, INF + winWidth };
		sphere[3].mat = rightWallMat;

		// ceil
		sphere[4].radius = INF;
		sphere[4].pos = cl_double3{ 0, INF + winHeight, 0 };
		sphere[4].mat = floorMat;

		// floor
		sphere[5].radius = INF;
		sphere[5].pos = cl_double3{ 0, -INF - winHeight, 0 };
		sphere[5].mat = floorMat;

		// back
		sphere[6].radius = INF;
		sphere[6].pos = cl_double3{ 2000 + INF, 0, 0 };
		sphere[6].mat = floorMat;

		// balls
		sphere[7].radius = 200;
		sphere[7].pos = cl_double3{ 1000, sphere[7].radius - winHeight, 350 };
		sphere[7].mat = metalMat;

		sphere[8].radius = 150;
		sphere[8].pos = cl_double3{ 1300, sphere[8].radius - winHeight, 100 };
		sphere[8].mat = diffuseMat;

		sphere[9].radius = 50;
		sphere[9].pos = cl_double3{ 800, sphere[9].radius - winHeight, 300 };
		sphere[9].mat = fuzzMetalMat;

		sphere[10].radius = 250;
		sphere[10].pos = cl_double3{ 1100, sphere[10].radius - winHeight, -225 };
		sphere[10].mat = dielectricMat;

		sphere[11].radius = 100;
		sphere[11].pos = cl_double3{ 800, sphere[11].radius - winHeight, -400 };
		sphere[11].mat = dielectricMat;
	}

	void configSharedData() {
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_ARRAY_BUFFER, pbo);
		glBufferData(GL_ARRAY_BUFFER, winWidth * winHeight * sizeof(cl_uint),
			NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		outBuffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, pbo, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create buffer from the PBO: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		sumBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(sum), sum, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create sumBuffer: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		camBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Camera), &cam, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create camBuffer: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		sphereBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sphereSize * sizeof(Sphere), sphere, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create sphereBuffer: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		cl_kernel kernel = kernels[kernalName];
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outBuffer);
		err &= clSetKernelArg(kernel, 1, sizeof(cl_mem), &sphereBuffer);
		err &= clSetKernelArg(kernel, 2, sizeof(cl_int), &sphereSize);
		err &= clSetKernelArg(kernel, 3, sizeof(cl_mem), &camBuffer);
		err &= clSetKernelArg(kernel, 6, sizeof(cl_mem), &sumBuffer);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't bind kernel arg: " << TranslateOpenCLError(err) << std::endl;
			return;
		}
	}

	void initGLBuffers() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertexCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void initTextures() {
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	void initShaders() {
		GLuint vs, fs, prog;
		char* vsSource, * fsSource;
		size_t vs_length, fs_length;

		vs = glCreateShader(GL_VERTEX_SHADER);
		fs = glCreateShader(GL_FRAGMENT_SHADER);

		ReadSourceFromFile("texture.vert", &vsSource, &vs_length);
		ReadSourceFromFile("texture.frag", &fsSource, &fs_length);

		glShaderSource(vs, 1, (const char**)&vsSource, (GLint*)&vs_length);
		glShaderSource(fs, 1, (const char**)&fsSource, (GLint*)&fs_length);

		compileShader(vs);
		compileShader(fs);

		delete[] vsSource;
		delete[] fsSource;

		prog = glCreateProgram();

		glBindAttribLocation(prog, 0, "in_coords");
		glBindAttribLocation(prog, 1, "in_color");

		glAttachShader(prog, vs);
		glAttachShader(prog, fs);

		glLinkProgram(prog);
		glUseProgram(prog);
	}

public:
	void setWidthAndHeight(int w, int h) {
		winWidth = w;
		winHeight = h;
	}

	void init() {
		srand(time(0));
		frameCount = 0;
		lstTime = clock();

		CLManager::init();

		std::vector<std::string> programFiles;
		programFiles.push_back("PathTrace.cl");
		createProgramFromFiles(programFiles);

		initGLBuffers();

		initTextures();

		initShaders();

		initData();

		configSharedData();
	}

	void runKernel() {
		cl_kernel kernel = kernels[kernalName];

		// par
		cl_uint seed = rand();
		frameCount++;
		err = clSetKernelArg(kernel, 4, sizeof(cl_uint), &seed);
		err &= clSetKernelArg(kernel, 5, sizeof(cl_ulong), &frameCount);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't bind kernel arg: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		size_t globalSize[]{ winWidth, winHeight };
		cl_event kernelEvent;

		glFinish();
		err = clEnqueueAcquireGLObjects(queue, 1, &outBuffer, 0, NULL, NULL);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't acquire the GL object" << std::endl;
			return;
		}

		err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalSize,
			nullptr, 0, nullptr, &kernelEvent);
		if (err != CL_SUCCESS) {
			std::cerr << "Run kernel failed: " << TranslateOpenCLError(err) << std::endl;
			return;
		}

		err = clWaitForEvents(1, &kernelEvent);
		if (err < 0) {
			std::cerr << "Couldn't wait for events" << std::endl;
			return;
		}

		clEnqueueReleaseGLObjects(queue, 1, &outBuffer, 0, NULL, NULL);
		clFinish(queue);
		clReleaseEvent(kernelEvent);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, winWidth, winHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	void render(GLFWwindow* window) {
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
		glfwSwapBuffers(window);

		clock_t nowTime = clock();
		sprintf(titleBuffer, "Ray Tracing Demo (%.3f FPS)", (double)CLOCKS_PER_SEC / (nowTime - lstTime));
		glfwSetWindowTitle(window, titleBuffer);
		lstTime = nowTime;
	}

	~GraphicManager() {
		clReleaseMemObject(outBuffer);
		clReleaseMemObject(sphereBuffer);
		clReleaseMemObject(camBuffer);
		clReleaseMemObject(sumBuffer);
		glDeleteBuffers(2, vbo);
	}
};