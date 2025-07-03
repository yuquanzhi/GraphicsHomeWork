//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include <thread>
#include <future>
struct RenderTask {
	int startY;
	int endY;
};
std::atomic<int> completedTasks_ = 0;

inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;
	int spp = 16;
	std::cout << "SPP: " << spp << "\n";
	//使用多线程来加速渲染
	auto renderTile = [&](int startRow, int endRow) {
		for (uint32_t j = startRow; j < endRow; ++j) {
			std::clog << "\r\033[KScanlines remaining: " << completedTasks_ / ((float)scene.width* (float)scene.height) << "%" << ' ' << std::flush;
			for (uint32_t i = 0; i < scene.width; ++i) {
				// generate primary ray direction
				float x = (2 * (i + 0.5) / (float)scene.width - 1) *
					imageAspectRatio * scale;
				float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
				Vector3f dir = normalize(Vector3f(-x, y, 1));
				for (int k = 0; k < spp; k++) {

					//Vector3f color = scene.castRayTest(Ray(eye_pos, dir), 0);

					framebuffer[j * scene.width + i] += scene.castRayTest(Ray(eye_pos, dir), 0) / spp;
				}
				completedTasks_++;
				//UpdateProgress(completedTasks_ /((float)scene.width * scene.height) );
			}
			//UpdateProgress(j / (float)scene.height);
		}
		};
	//用多线程进行对应的渲染
	const int numThreads = std::thread::hardware_concurrency();
	const int tileHeight = std::max(32, scene.height / (numThreads * 4));
	std::vector<std::thread> threads;
	std::vector<std::future<void>> futures;
	std::vector<RenderTask> tasks;
	for (int y = 0; y < scene.height; y += tileHeight) {
		tasks.push_back({ y, std::min(y + tileHeight,scene.height) });
	}
	// 使用无锁队列分发任务
	std::atomic<int> nextTask(0);

	for (int i = 0; i < numThreads; ++i) {
		futures.push_back(std::async(std::launch::async, [&]() {
			while (true) {
				int taskIdx = nextTask++;
				if (taskIdx >= tasks.size()) break;

				auto& task = tasks[taskIdx];
				renderTile(task.startY, task.endY);
			}
			}));
	}
	// 等待所有任务完成
	for (auto& future : futures) {
		future.wait();
	}


     //change the spp value to change sample ammount
    
    //for (uint32_t j = 0; j < scene.height; ++j) {
    //    for (uint32_t i = 0; i < scene.width; ++i) {
    //        // generate primary ray direction
    //        float x = (2 * (i + 0.5) / (float)scene.width - 1) *
    //                  imageAspectRatio * scale;
    //        float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

    //        Vector3f dir = normalize(Vector3f(-x, y, 1));
    //        for (int k = 0; k < spp; k++){
    //            framebuffer[m] += scene.castRayTest(Ray(eye_pos, dir), 0) / spp;
    //        }
    //        m++;
    //    }
    //    UpdateProgress(j / (float)scene.height);
    //}
    //UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
