__constant double EPS = 1e-6;

typedef struct Ray {
	double3 pos;
	double3 dir;
} Ray;

typedef struct Cam {
	double theta;
	double width;
	double height;
	double3 pos;
	double3 up;
	double3 lookAt;
} Cam;

typedef struct Material {
	double refraction;
	double reflection;
	double diffuse;
	double3 color;
	double3 emission;
} Material;

typedef struct Sphere {
	double radius;
	double3 pos;
	Material mat;
} Sphere;

Ray getPixelRay(__constant Cam* cam, int x, int y) {
	Ray ret;
	double3 w = -normalize(cam->lookAt);
	double3 v = normalize(cam->up);
	double3 u = cross(v, w);
	double halfHeight = cam->height / 2;
	double halfWidth = cam->width / 2;
	double distance = halfHeight / tan(cam->theta / 2);
	double3 eyePos = cam->pos + w * distance;
	double3 leftBottomPos = cam->pos - v * halfHeight - u * halfWidth;
	double tu = (double)x / get_global_size(0);
	double tv = 1.0 - (double)y / get_global_size(1);

	ret.pos = leftBottomPos + tu * cam->width * u + tv * cam->height * v;
	ret.dir = normalize(ret.pos - eyePos);

	return ret;
}

double getFirstCollideWithSphere(const Ray* ray, const Sphere* sphere) {
	double a = pow(ray->dir.x, 2) + pow(ray->dir.y, 2) + pow(ray->dir.z, 2);
	double b = 2 * (ray->dir.x * (ray->pos.x - sphere->pos.x)
		+ ray->dir.y * (ray->pos.y - sphere->pos.y)
		+ ray->dir.z * (ray->pos.z - sphere->pos.z));
	double c = pow(ray->pos.x - sphere->pos.x, 2)
		+ pow(ray->pos.y - sphere->pos.y, 2)
		+ pow(ray->pos.z - sphere->pos.z, 2)
		- pow(sphere->radius, 2);
	double delta = b * b - 4 * a * c;
	if (delta < 0) return -1;
	delta = sqrt(delta);
	double t = (-b - delta) / (2 * a);
	if (t >= EPS) return t;
	t = (-b + delta) / (2 * a);
	if (t >= EPS) return t;
	return -1;
}

double3 getFirstCollide(const Ray* ray, __constant Sphere* sphere, const int sphereSize, int* id) {
	*id = -1;
	double mm = 0;
	for (int i = 0; i < sphereSize; i++)
	{
		Sphere nows = sphere[i];
		double t = getFirstCollideWithSphere(ray, &nows);
		if (t == -1) continue;
		if (mm == 0 || t < mm) {
			mm = t;
			*id = i;
		}
	}
	return ray->pos + mm * ray->dir;
}

double3 emitRay(Ray ray, __constant Sphere* sphere, const int sphereSize) {
	int id = -1, lightId = -1;
	double3 color = (double3)(0, 0, 0);

	double3 pos = getFirstCollide(&ray, sphere, sphereSize, &id);
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
	else if (lightId == id) return sphere[lightId].mat.emission;
	Sphere light = sphere[lightId];

	// shadow
	Ray shadowRay;
	shadowRay.dir = normalize(light.pos - pos);
	shadowRay.pos = pos;
	int collideId = -1;
	getFirstCollide(&shadowRay, sphere, sphereSize, &collideId);
	if (collideId != lightId) return color;

	double3 lightPos = light.pos;
	double3 lightCol = light.mat.emission;

	double3 nd = normalize(pos - sphere[id].pos);
	double3 ld = normalize(lightPos - pos);
	double3 vd = -ray.dir;
	double3 hd = normalize(ld + vd);
	double3 specular = pow(dot(nd, hd), 10) * sphere[id].mat.reflection * lightCol;
	double3 diffuse = dot(ld, nd) * sphere[id].mat.color * lightCol;

	color += diffuse + specular;

	return color;
}

__kernel void kernelMain(__global uchar3* pixels, __constant Sphere* sphere, const int sphereSize, __constant Cam* cam) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	uint idx = coord.y * get_global_size(0) + coord.x;

	Ray startRay = getPixelRay(cam, coord.x, coord.y);

	double3 color = emitRay(startRay, sphere, sphereSize);

	pixels[idx].x = (float)color.x * 255;
	pixels[idx].y = (float)color.y * 255;
	pixels[idx].z = (float)color.z * 255;
}