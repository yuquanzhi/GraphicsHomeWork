//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

	//Ë«ÏßÐÔ²åÖµ
	Eigen::Vector3f getBilinearColor(float u, float v)
	{
		auto u_img = u * width;
		auto v_img = (1 - v) * height;
		int x1 = static_cast<int>(u_img);
		int y1 = static_cast<int>(v_img);
		int x2 = std::min(x1 + 1, width - 1);
		int y2 = std::min(y1 + 1, height - 1);
		float x_diff = u_img - x1;
		float y_diff = v_img - y1;
		Eigen::Vector3f c11 = getColor(x1 / static_cast<float>(width), y1 / static_cast<float>(height));
		Eigen::Vector3f c12 = getColor(x1 / static_cast<float>(width), y2 / static_cast<float>(height));
		Eigen::Vector3f c21 = getColor(x2 / static_cast<float>(width), y1 / static_cast<float>(height));
		Eigen::Vector3f c22 = getColor(x2 / static_cast<float>(width), y2 / static_cast<float>(height));
		return (c11 * (1 - x_diff) * (1 - y_diff) +
			c21 * x_diff * (1 - y_diff) +
			c12 * (1 - x_diff) * y_diff +
			c22 * x_diff * y_diff);
	}

};
#endif //RASTERIZER_TEXTURE_H
