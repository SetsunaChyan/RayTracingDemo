__constant double EPS = 1e-3;

typedef ulong llu;

typedef struct Seed64 {
	llu k1, k2;
} Seed64;

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
	double refractionCoefficient;
	double reflectionWeight;
	int type;
	double3 color;
} Material;

typedef struct Sphere {
	double radius;
	double3 pos;
	Material mat;
} Sphere;

static llu rand64(Seed64* seed)
{
	llu k3 = seed->k1, k4 = seed->k2;
	seed->k1 = k4;
	k3 ^= k3 << 11;
	seed->k2 = k3 ^ k4 ^ (k3 >> 8) ^ (k4 >> 13);
	return seed->k2 + k4;
}

static double rand(Seed64* seed) {
	return (double)rand64(seed) / (-1lu);
}

static double norm2(double3 v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

Ray getPixelRay(__constant Cam* cam, int x, int y, Seed64* seed) {
	Ray ret;
	double3 w = -normalize(cam->lookAt);
	double3 v = normalize(cam->up);
	double3 u = cross(v, w);
	double halfHeight = cam->height / 2;
	double halfWidth = cam->width / 2;
	double distance = halfHeight / tan(cam->theta / 2);
	double3 eyePos = cam->pos + w * distance;
	double3 leftBottomPos = cam->pos - v * halfHeight - u * halfWidth;
	double tu = (x + rand(seed)) / get_global_size(0);
	double tv = 1.0 - (y + rand(seed)) / get_global_size(1);

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
	if (delta <= 0) return -1;
	delta = sqrt(delta);
	double t = (-b - delta) / (2 * a);
	if (t > EPS) return t;
	t = (-b + delta) / (2 * a);
	if (t > EPS) return t;
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

static double3 rand3(Seed64* seed) {
	double3 ret;
	do {
		ret = 2 * (double3)(rand(seed), rand(seed), rand(seed)) - (double3)(1, 1, 1);
	} while (norm2(ret) >= 1);
	return normalize(ret);
}

double3 reflect(const double3 id, const double3 nd) {
	return id - 2.0 * dot(id, nd) * nd;
}

double3 refract(const double3 id, const double3 nd, const double co) {
	double cosTheta = min(dot(-id, nd), 1.0);
	double3 ra = co * (id + cosTheta * nd);
	double3 rb = -sqrt(fabs(1.0 - dot(ra, ra))) * nd;
	return ra + rb;
}

double3 emitRay(Ray ray, __constant Sphere* sphere, const int sphereSize, Seed64* seed) {
	const double P = 0.8;
	const int maxDep = 10;
	Sphere o;
	int id;
	bool isFront;
	double3 color = (double3)(0, 0, 0);
	double3 brightness = (double3)(1, 1, 1);

	for (int i = 0; i < maxDep; i++) {
		if (rand(seed) > P) break;
		double3 pos = getFirstCollide(&ray, sphere, sphereSize, &id);
		if (id == -1) break;

		o = sphere[id];
		if (o.mat.type == 0) {
			color = o.mat.color * brightness;
			break;
		}

		bool isFront = (dot(ray.dir, pos - o.pos) < 0);
		double3 nd = normalize(pos - o.pos);

		ray.pos = pos;
		brightness *= o.mat.color;
		if (o.mat.type == 1) {
			ray.dir = normalize(rand3(seed) + nd);
		} else if (o.mat.type == 2 || o.mat.type == 4) {
			double fuzz = 0.0;
			if (o.mat.type == 4) fuzz = 0.4;
			ray.dir = normalize(reflect(ray.dir, nd) + fuzz * rand3(seed));
			if (dot(ray.dir, nd) < 0) break;
		} else if (o.mat.type == 3) {
			double co = o.mat.refractionCoefficient;
			if (isFront) co = 1.0 / co; else nd = -nd;
			double cosTheta = min(dot(-ray.dir, nd), 1.0);
			double sinTheta = sqrt(1 - pow(cosTheta, 2));
			bool isReflect = false;
			if (co * sinTheta > 1) isReflect = true;
			else {
				double R = pow((1 - co) / (1 + co), 2);
				R += (1 - R) * pow(1 - cosTheta, 5);
				if (rand(seed) < R) isReflect = true;
			}
			if (isReflect) {
				ray.dir = reflect(ray.dir, nd);
			} else {
				ray.dir = normalize(refract(normalize(ray.dir), nd, co));
			}
		}
		brightness /= P;
	}
	return color;
}

__kernel void kernelMain(__global uchar3* pixels, __constant Sphere* sphere, const int sphereSize, __constant Cam* cam,
	const uint Seed, const llu frame, __global double3* sumColor) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	uint idx = coord.y * get_global_size(0) + coord.x;

	// random seed
	Seed64 seed;
	llu magic = coord.x * (1e9 + 7) + (coord.x * coord.y) * (1e9 + 9) + 998244353;
	seed.k1 = (llu)(Seed ^ magic) * (Seed ^ magic);
	seed.k2 = (llu)magic * magic * (1e5 + 7);

	Ray startRay = getPixelRay(cam, coord.x, coord.y, &seed);

	int sampleNum = 1;
	double3 color = (double3)(0, 0, 0);
	for (int i = 0; i < sampleNum; i++)
		color += emitRay(startRay, sphere, sphereSize, &seed);
	color /= sampleNum;

	sumColor[idx] += color;
	color = sqrt(sumColor[idx] / frame);
	pixels[idx].x = (float)color.x * 255;
	pixels[idx].y = (float)color.y * 255;
	pixels[idx].z = (float)color.z * 255;
}