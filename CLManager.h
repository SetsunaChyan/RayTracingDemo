#pragma once
#include <CL/opencl.h>
#include <iostream>
#include <unordered_map>

#include "util.h"
#include "BasicDS.h"

class CLManager {
protected:
	cl_int err;

private:
	void initPlatform() {
		err = clGetPlatformIDs(1, &platform, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Cannot get platform: " << TranslateOpenCLError(err) << std::endl;
			return;
		}
	}

	void initDevice() {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Cannot get device: " << TranslateOpenCLError(err) << std::endl;
			return;
		}
	}

	void initContext() {
		cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
			0
		};
		context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Create context failed: " << TranslateOpenCLError(err) << std::endl;
			return;
		}
	}

	void initQueue() {
		queue = clCreateCommandQueue(context, device, 0, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Create command queue failed: " << TranslateOpenCLError(err) << std::endl;
			return;
		}
	}

public:
	cl_platform_id platform = 0;
	cl_device_id device = 0;
	cl_context context = 0;
	cl_command_queue queue = 0;
	cl_program program = 0;
	std::unordered_map<std::string, cl_kernel> kernels;

	void init() {
		err = 0;
		initPlatform();
		initDevice();
		initContext();
		initQueue();
		program = 0;
	}

	bool createKernels() {
		cl_uint num = 0;
		err = clCreateKernelsInProgram(program, 0, nullptr, &num);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create kernels: " << TranslateOpenCLError(err) << std::endl;
			return false;
		}

		cl_kernel* tmpKernels = new cl_kernel[num];
		const int bufferSize = 256;
		char* kernalName = new char[bufferSize];
		clCreateKernelsInProgram(program, num, tmpKernels, nullptr);
		for (int i = 0; i < num; i++) {
			clGetKernelInfo(tmpKernels[i], CL_KERNEL_FUNCTION_NAME, bufferSize, kernalName, nullptr);
			kernels[kernalName] = tmpKernels[i];
		}

		delete[] kernalName;

		return true;
	}

	bool createProgramFromFiles(const std::vector<std::string>& fileNames) {
		if (program) clReleaseProgram(program);

		size_t num = fileNames.size();
		char** buffers = new char* [num];
		size_t* lens = new size_t[num];
		for (int i = 0; i < num; i++) {
			err = ReadSourceFromFile(fileNames[i].c_str(), &(buffers[i]), &(lens[i]));
			if (err != CL_SUCCESS)
			{
				std::cerr << "Read source from file \"" << fileNames[i] << "\" failed: "
					<< TranslateOpenCLError(err) << std::endl;
				return false;
			}
		}

		program = clCreateProgramWithSource(context, num, (const char**)buffers, lens, &err);
		if (err != CL_SUCCESS) {
			std::cerr << "Couldn't create the program: " << TranslateOpenCLError(err) << std::endl;
			return false;
		}

		err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			size_t logSize;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
			char* programLog = new char[logSize + 1];
			programLog[logSize] = 0;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, programLog, &logSize);
			std::cerr << programLog << std::endl;
			delete[] programLog;
			return false;
		}

		for (int i = 0; i < num; i++) delete[] buffers[i];
		delete[] buffers;
		delete[] lens;

		if (!createKernels()) return false;

		return true;
	}

	void clearKernels() {
		for (auto& it : kernels)
			if (it.second) clReleaseKernel(it.second);
	}

	~CLManager() {
		clearKernels();
		clReleaseProgram(program);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		clReleaseDevice(device);
	}
};
