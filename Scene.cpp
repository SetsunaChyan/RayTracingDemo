#include "Scene.h"

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

void initScene1(Camera& cam, Sphere sphere[], int& sphereSize, int winWidth, int winHeight) {
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

void initScene2(Camera& cam, Sphere sphere[], int& sphereSize, int winWidth, int winHeight) {
	cam.pos = cl_double3{ 0.0,0.0,0.0 };
	cam.up = cl_double3{ 0.0,1.0,0.0 };
	cam.lookAt = cl_double3{ 1.0,0.0,0.0 };
	cam.theta = CL_M_PI / 3.0;
	cam.winWidth = winWidth;
	cam.winHeight = winHeight;

	Material metalMat, dielectricMat;
	Material lightMat, backMat, floorMat;
	metalMat.color = cl_double3{ 0xFF,0xFF,0xFF } / 256.0;
	metalMat.refraction = 0;
	metalMat.reflection = 0;
	metalMat.type = 2;

	dielectricMat.color = cl_double3{ 0xFF,0xFF,0xFF } / 256.0;
	dielectricMat.refraction = 1.3;
	dielectricMat.reflection = 0;
	dielectricMat.type = 3;

	lightMat.color = cl_double3{ 0xFF,0xFF,0xFF } / (256.0 / 15.0);
	lightMat.refraction = 0;
	lightMat.reflection = 0;
	lightMat.type = 0;

	backMat.color = cl_double3{ 0x66,0xCC,0xFF } / 256.0;
	backMat.refraction = backMat.reflection = 0;
	backMat.type = 1;
	floorMat = backMat;
	//floorMat.color = cl_double3{ 0xDD,0x00,0x00 } / 256.0;
	floorMat.color = cl_double3{ 0xFF,0xFF,0xFF } / 256.0;

	sphereSize = 6;
	const cl_double INF = 1e6;
	// light
	sphere[0].radius = 1000;
	sphere[0].pos = cl_double3{ -2000, sphere[0].radius + winHeight, 0 };
	sphere[0].mat = lightMat;

	// floor
	sphere[1].radius = INF;
	sphere[1].pos = cl_double3{ 0, -INF - winHeight, 0 };
	sphere[1].mat = floorMat;

	// back
	sphere[2].radius = INF;
	sphere[2].pos = cl_double3{ 3000 + INF, 0, 0 };
	sphere[2].mat = backMat;

	// ceil
	sphere[3].radius = INF;
	sphere[3].pos = cl_double3{ 0, INF +2*winWidth, 0 };
	sphere[3].mat = floorMat;

	// balls
	sphere[4].radius = 200;
	sphere[4].pos = cl_double3{ 700, -200, 100 };
	sphere[4].mat = metalMat;

	sphere[5].radius = 200;
	sphere[5].pos = cl_double3{ 500, 0, 0 };
	sphere[5].mat = dielectricMat;
}