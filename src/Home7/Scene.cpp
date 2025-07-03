//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

#include "Intersection.hpp"

void Scene::buildBVH() {
	printf(" - Generating BVH...\n\n");
	this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray& ray) const
{
	return this->bvh->Intersect(ray);
}



void Scene::sampleLight(Intersection& pos, float& pdf) const
{
	float emit_area_sum = 0;
	for (uint32_t k = 0; k < objects.size(); ++k) {
		if (objects[k]->hasEmit()) {
			emit_area_sum += objects[k]->getArea();
		}
	}
	float p = get_random_float() * emit_area_sum;
	emit_area_sum = 0;
	for (uint32_t k = 0; k < objects.size(); ++k) {
		if (objects[k]->hasEmit()) {
			emit_area_sum += objects[k]->getArea();
			if (p <= emit_area_sum) {
				objects[k]->Sample(pos, pdf);
				break;
			}
		}
	}
}

bool Scene::trace(
	const Ray& ray,
	const std::vector<Object*>& objects,
	float& tNear, uint32_t& index, Object** hitObject) const
{
	*hitObject = nullptr;
	for (uint32_t k = 0; k < objects.size(); ++k) {
		float tNearK = kInfinity;
		uint32_t indexK;
		Vector2f uvK;
		if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
			*hitObject = objects[k];
			tNear = tNearK;
			index = indexK;
		}
	}


	return (*hitObject != nullptr);
}





// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray& ray, int depth) const
{
	Intersection intersection = this->intersect(ray);
	if (!intersection.happened)
	{
		return backgroundColor;
	}
	if (intersection.m->hasEmission())
	{
		return intersection.m->getEmission();
	}

	Intersection light_intersection;
	Vector3f direct_lighting(0.f);
	float pdf ;
	Vector3f N = intersection.normal.normalized();
	this->sampleLight(light_intersection, pdf);


	//ws
	Vector3f light_dir = normalize(light_intersection.coords - intersection.coords);
	//NN
	Vector3f light_normal = light_intersection.normal.normalized();


	float light_distance = (light_intersection.coords - intersection.coords).norm();

	Vector3f shadow_orig = intersection.coords + light_dir * EPSILON;
	Ray shadow_ray(shadow_orig, light_dir);


	//float tNear = light_distance - EPSILON;
	//uint32_t index = 0;
	//Object* hitObject = nullptr;
	Intersection testLightHit= this->intersect(shadow_ray);
	Vector3f wo = ray.direction;

	Vector3f ws = light_dir;

	Vector3f normal = intersection.normal.normalized();

	if (testLightHit.happened && testLightHit.obj->hasEmit())
	{
		

		direct_lighting = light_intersection.emit * intersection.m->eval(wo, ws, N) *
			dotProduct(ws, N) * dotProduct(-ws, light_normal) /
			(light_distance * light_distance) / pdf;
	}


	//return direct_lighting;

	Vector3f indirect_lighting(0.f);

	float P_RR = get_random_float();
	if (P_RR < RussianRoulette) {
		Vector3f wi = intersection.m->sample(wo, N).normalized();
		Ray r(intersection.coords, wi);
		Intersection inter = intersect(r);
		// 判断打到的物体是否会发光取决于m
		if (inter.happened && !inter.m->hasEmission()) {
			Vector3f eval = intersection.m->eval(wo, wi, N);
			float pdf_O = intersection.m->pdf(wo, wi, N);
			float cos_theta = dotProduct(wi, N);
			Vector3f color = castRay(r, depth + 1);
			indirect_lighting = color * eval * cos_theta / pdf_O / RussianRoulette;
		}
	}






	//开始俄罗斯轮盘赌
	//生成一个随机数
	//float p = get_random_float();

	//if (p < RussianRoulette)
	//{

	//	//wi
	//	Vector3f reflection_dir =  intersection.m->sample(wo, N).normalized();
	//	Ray reflection_ray(intersection.coords + reflection_dir * EPSILON, reflection_dir);


	//	Intersection reflection_intersection = this->intersect(reflection_ray);
	//	if (reflection_intersection.happened && !reflection_intersection.m->hasEmission())
	//	{

	//		//wi = normalize(reflection_intersection.coords - intersection.coords);

	//		//Vector3f N = intersection.normal;
	//		Vector3f eval = intersection.m->eval(wo, reflection_dir, N);
	//		float pdf_O = intersection.m->pdf(wo, reflection_dir, N);
	//		float cos_theta = dotProduct(reflection_dir, N);
	//		Vector3f color = this->castRay(reflection_ray, depth + 1);
	//		indirect_lighting = color * eval *
	//			cos_theta / pdf_O / RussianRoulette;

	//	}
	//}
	//返回直接光照和间接光照的和
	return direct_lighting+ indirect_lighting;
	//return Vector3f(0.0, 0.0, 0.0); // Placeholder for the actual implementation
}
Vector3f Scene::castRayTest(const Ray& ray, int depth) const
{
	// TO DO Implement Path Tracing Algorithm here

	// ray is the wo_ray
	// p_inter is the intersection between ray and object plane
	// x_inter is the intersection between ray and light plane
	// ray from p to x is ws_ray


	Intersection p_inter = intersect(ray);
	if (!p_inter.happened) {
		return Vector3f();
	}
	if (p_inter.m->hasEmission()) {
		return p_inter.m->getEmission();
	}

	float EPLISON = 0.0001;
	Vector3f l_dir;
	Vector3f l_indir;

	// sampleLight(inter, pdf_light)
	Intersection x_inter;
	float pdf_light = 0.0f;
	sampleLight(x_inter, pdf_light);

	// Get x, ws, NN, emit from inter
	Vector3f p = p_inter.coords;
	Vector3f x = x_inter.coords;
	Vector3f ws_dir = (x - p).normalized();
	float ws_distance = (x - p).norm();
	Vector3f N = p_inter.normal.normalized();
	Vector3f NN = x_inter.normal.normalized();
	Vector3f emit = x_inter.emit;

	// Shoot a ray from p to x
	Ray ws_ray(p, ws_dir);
	Intersection ws_ray_inter = intersect(ws_ray);
	// If the ray is not blocked in the middle
	if (ws_ray_inter.happened&& ws_ray_inter.obj->hasEmit()) {
		l_dir = emit * p_inter.m->eval(ray.direction, ws_ray.direction, N)
			* dotProduct(ws_ray.direction, N)
			* dotProduct(-ws_ray.direction, NN)
			/ std::pow(ws_distance, 2)
			/ pdf_light;
	}

	// Test Russian Roulette with probability RussianRoulette
	if (get_random_float() > RussianRoulette) {
		return l_dir;
	}

	l_indir = 0.0;

	Vector3f wi_dir = p_inter.m->sample(ray.direction, N).normalized();
	Ray wi_ray(p_inter.coords, wi_dir);
	// If ray r hit a non-emitting object at q
	Intersection wi_inter = intersect(wi_ray);
	if (wi_inter.happened && (!wi_inter.m->hasEmission())) {

		Vector3f color = castRayTest(wi_ray, depth + 1);


		l_indir = color * p_inter.m->eval(ray.direction, wi_ray.direction, N)
			* dotProduct(wi_ray.direction, N)
			/ p_inter.m->pdf(ray.direction, wi_ray.direction, N)
			/ RussianRoulette;

		
	}

	return  l_dir+l_indir;

}