#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
	Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

	Eigen::Matrix4f translate;
	translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
		-eye_pos[2], 0, 0, 0, 1;

	view = translate * view;

	return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
	Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

	// TODO: Implement this function
	// Create the model matrix for rotating the triangle around the Z axis.
	// Then return it.
	Eigen::Matrix4f translate;
	translate << cos(rotation_angle / 180 * MY_PI), -sin(rotation_angle / 180 * MY_PI), 0, 0,
		sin(rotation_angle / 180 * MY_PI), cos(rotation_angle / 180 * MY_PI), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;
	model = translate * model;

	return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
	float zNear, float zFar)
{
	// Students will implement this function

	Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();


	// TODO: Implement this function
	// Create the projection matrix for the given parameters.
	// Then return it.
	//设置对应的正交矩阵 length,width,height;
	float t = zNear * tan(eye_fov / 2 * MY_PI / 180);
	float r = t * aspect_ratio;
	float l = -r;
	float b = -t;
	float n = zNear;
	float f = zFar;
	Eigen::Matrix4f orthographic;
	orthographic << 1 / r, 0, 0, 0,
		0, 1 / t, 0, 0,
		0, 0, 2 / ( zFar- zNear), 0,
		0, 0, 0, 1;
	Eigen::Matrix4f translate;
	translate << 1, 0, 0, -(r + l) / 2.0f,
		0, 1, 0, -(t + b) / 2.0f,
		0, 0, 1, -(n + f) / 2.0f,
		0, 0, 0, 1;
	orthographic = orthographic * translate;

	//把透视投影矩阵压缩转换为正交投影矩阵
	projection << zNear, 0, 0, 0,
		0, zNear, 0, 0,
		0, 0, zNear+ zFar, -zNear* zFar,
		0, 0,1, 0;

	projection = orthographic * projection;

	return projection;
}

Eigen::Matrix4f get_rotation(Eigen::Vector3f  axis, float anger)
{
	axis.normalize();
	Eigen::AngleAxisf rotation(anger / 180 * MY_PI, axis);
	Eigen::Matrix3f rotation_matrix = rotation.toRotationMatrix();

	Eigen::Matrix4f rotation_4f = Eigen::Matrix4f::Identity();
	rotation_4f.block<3, 3>(0, 0) = rotation_matrix;
	return rotation_4f;

}

int main(int argc, const char** argv)
{
	float angle = 0;
	bool command_line = false;
	std::string filename = "output.png";

	if (argc >= 3) {
		command_line = true;
		angle = std::stof(argv[2]); // -r by default
		if (argc == 4) {
			filename = std::string(argv[3]);
		}
		else
			return 0;
	}

	rst::rasterizer r(700, 700);

	Eigen::Vector3f eye_pos = { 0, 0, 5 };

	std::vector<Eigen::Vector3f> pos{ {2, 0, -2}, {0, 2, -2}, {-2, 0, -2} };

	std::vector<Eigen::Vector3i> ind{ {0, 1, 2} };

	auto pos_id = r.load_positions(pos);
	auto ind_id = r.load_indices(ind);

	int key = 0;
	int frame_count = 0;

	int rotation_anger = 0; // 0 for x, 1 for y, 2 for z
	Eigen::Vector3f rotation_axis = { 1, 1, 1 }; // default rotation around z-axis10, 0, 1 }; // default rotation around z-axis

	if (command_line) {
		r.clear(rst::Buffers::Color | rst::Buffers::Depth);

		r.set_model(get_model_matrix(angle));
		r.set_view(get_view_matrix(eye_pos));
		r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

		r.draw(pos_id, ind_id, rst::Primitive::Triangle);
		cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
		image.convertTo(image, CV_8UC3, 1.0f);

		cv::imwrite(filename, image);

		return 0;
	}
	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	while (key != 27) {
		r.clear(rst::Buffers::Color | rst::Buffers::Depth);

		Eigen::Matrix4f model_matrix = get_model_matrix(angle);
		model_matrix = model_matrix * get_rotation(rotation_axis, rotation_anger);

		r.set_model(model_matrix);
		r.set_view(get_view_matrix(eye_pos));
		r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

		r.draw(pos_id, ind_id, rst::Primitive::Triangle);

		cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
		image.convertTo(image, CV_8UC3, 1.0f);
		cv::imshow("image", image);
		key = cv::waitKey(10);

		//std::cout << "frame count: " << frame_count++ << '\n';
		//std::cout << "Key On the way" << key << std::endl;

		if (key == 'a') {
			angle += 10;
			//std::cout << "A  Key On the way" << key << std::endl;
		}
		else if (key == 'd') {
			angle -= 10;
		}
		if (key == 'q') {
			rotation_anger += 10;
			std::cout << "q  Key On the way" << rotation_anger << std::endl;
		}
		else if (key == 'e') {
			rotation_anger -= 10;
		}
	}

	return 0;
}
