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
	if (ws_ray_inter.happened && ws_ray_inter.obj->hasEmit()) {
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

	return  l_dir + l_indir;

}