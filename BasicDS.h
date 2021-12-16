#pragma once
#include <CL/opencl.h>

//__declspec(align(16))
struct Camera {
	cl_double theta;
	cl_double winWidth;
	cl_double winHeight;
	cl_double3 __declspec(align(16)) pos; // 屏幕正中间的位置
	cl_double3 up;
	cl_double3 lookAt;
};

struct Material {
	cl_double refraction;
	cl_double reflection;
	/*
	* 0: light
	* 1: diffuse
	* 2: metal
	* 3: dieletric
	* 4: fuzz metal
	*/
	cl_int type;
	cl_double3 __declspec(align(16)) color;
};

struct Sphere {
	cl_double radius;
	cl_double3 __declspec(align(32)) pos;
	Material mat;
};

cl_double3& operator /= (cl_double3& o1, const double o2)
{
	o1.x /= o2, o1.y /= o2, o1.z /= o2;
	return o1;
}

cl_double3 operator / (const cl_double3 o1, const double o2)
{
	cl_double3 ret = o1;
	ret.x /= o2, ret.y /= o2, ret.z /= o2;
	return ret;
}