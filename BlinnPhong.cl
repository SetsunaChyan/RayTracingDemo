__constant float EPS = 1e-3;

typedef struct Ray {
	float3 pos;
	float3 dir;
} Ray;

typedef struct Cam {
	float theta;
	float width;
	float height;
	float3 pos;
	float3 up;
	float3 lookAt;
} Cam;

typedef struct Material {
	float refraction;
	float reflection;
	float diffuse;
	float3 color;
	float3 emission;
} Material;

typedef struct Sphere {
	float radius;
	float3 pos;
	Material mat;
} Sphere;

Ray getPixelRay(__constant Cam* cam, int x, int y) {
	Ray ret;
	float3 w = -normalize(cam->lookAt);
	float3 v = normalize(cam->up);
	float3 u = cross(v, w);
	float halfHeight = cam->height / 2;
	float halfWidth = cam->width / 2;
	float distance = halfHeight / tan(cam->theta / 2);
	float3 eyePos = cam->pos + w * distance;
	float3 leftBottomPos = cam->pos - v * halfHeight - u * halfWidth;
	float tu = (float)x / get_global_size(0);
	float tv = 1.0f - (float)y / get_global_size(1);

	ret.pos = leftBottomPos + tu * cam->width * u + tv * cam->height * v;
	ret.dir = normalize(ret.pos - eyePos);

	return ret;
}

float getFirstCollideWithSphere(const Ray* ray, const Sphere* sphere) {
	float a = pow(ray->dir.x, 2) + pow(ray->dir.y, 2) + pow(ray->dir.z, 2);
	float b = 2 * (ray->dir.x * (ray->pos.x - sphere->pos.x)
		+ ray->dir.y * (ray->pos.y - sphere->pos.y)
		+ ray->dir.z * (ray->pos.z - sphere->pos.z));
	float c = pow(ray->pos.x - sphere->pos.x, 2)
		+ pow(ray->pos.y - sphere->pos.y, 2)
		+ pow(ray->pos.z - sphere->pos.z, 2)
		- pow(sphere->radius, 2);
	float delta = b * b - 4 * a * c;
	if (delta <= 0) return -1;
	delta = sqrt(delta);
	float t = (-b - delta) / (2 * a);
	if (t > EPS) return t;
	t = (-b + delta) / (2 * a);
	if (t > EPS) return t;
	return -1;
}

float3 getFirstCollide(const Ray* ray, __constant Sphere* sphere, const int sphereSize, int* id) {
	*id = -1;
	float mm = 0;
	for (int i = 0; i < sphereSize; i++)
	{
		Sphere nows = sphere[i];
		float t = getFirstCollideWithSphere(ray, &nows);
		if (t == -1) continue;
		if (mm == 0 || t < mm) {
			mm = t;
			*id = i;
		}
	}
	return ray->pos + mm * ray->dir;
}

float3 emitRay(Ray ray, __constant Sphere* sphere, const int sphereSize) {
	int id = -1, lightId = -1;
	float3 color = (float3)(0, 0, 0);

	float3 pos = getFirstCollide(&ray, sphere, sphereSize, &id);
	if (id == -1) return color;

	for (int i = 0; i < sphereSize; i++) {
		if (sphere[i].mat.emission.x != 0 ||
			sphere[i].mat.emission.y != 0 ||
			sphere[i].mat.emission.z != 0) {
			lightId = i;
			break;
		}
	}
	if (lightId == -1) return color;

	float3 lightPos = sphere[lightId].pos;
	float3 lightCol = sphere[lightId].mat.emission;

	float3 nd = normalize(pos - sphere[id].pos);
	float3 ld = normalize(lightPos - pos);
	float3 vd = -ray.dir;
	float3 hd = normalize(ld + vd);
	float3 specular = pow(max(0.0f, dot(nd, hd)), 10) * sphere[id].mat.reflection * lightCol;
	float3 diffuse = dot(ld, nd) * sphere[id].mat.color * lightCol;

	color += diffuse + specular;

	return color;
}

__kernel void kernelMain(__global uchar3* pixels, __constant Sphere* sphere, const int sphereSize, __constant Cam* cam) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	uint idx = coord.y * get_global_size(0) + coord.x;

	Ray startRay = getPixelRay(cam, coord.x, coord.y);

	float3 color = emitRay(startRay, sphere, sphereSize);

	pixels[idx].x = color.x * 255;
	pixels[idx].y = color.y * 255;
	pixels[idx].z = color.z * 255;
}