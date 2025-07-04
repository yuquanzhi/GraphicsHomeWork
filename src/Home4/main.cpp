#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;

		//对于像素相邻的像素 需要计算点对应的颜色
		
        int x = static_cast<int>(std::floor( point.x));
		int y = static_cast<int>(std::floor(point.y));
		//对向上的像素进行赋值
        if (y - 1 >= 0)
        {
            float temp= window.at<cv::Vec3b>(y - 1, x)[2];
			//计算和当前点的距离
			float distance = std::sqrt(std::pow(point.x - x-0.5, 2) + std::pow(point.y - (y - 1)-0.5, 2));
			window.at<cv::Vec3b>(y - 1, x)[2] = static_cast<uchar>(std::max(temp,  (1 - distance) * 255));

        }
		//对向下的像素进行赋值
        if (y + 1 < window.rows)
        {
            float temp = window.at<cv::Vec3b>(y + 1, x)[2];
            float distance = std::sqrt(std::pow(point.x - x - 0.5, 2) + std::pow(point.y - (y + 1) - 0.5, 2));
            window.at<cv::Vec3b>(y + 1, x)[2] = static_cast<uchar>(std::max(temp, (1 - distance) * 255));

        }
		//对向左的像素进行赋值
		if (x - 1 >= 0)
		{
			float temp = window.at<cv::Vec3b>(y, x - 1)[2];
			float distance = std::sqrt(std::pow(point.x - (x - 1) - 0.5, 2) + std::pow(point.y - y - 0.5, 2));
			window.at<cv::Vec3b>(y, x - 1)[2] = static_cast<uchar>(std::max(temp, (1 - distance) * 255));
		}
		//对向右的像素进行赋值
        if (x + 1 < window.cols)
        {
            float temp = window.at<cv::Vec3b>(y, x + 1)[2];
            float distance = std::sqrt(std::pow(point.x - (x + 1) - 0.5, 2) + std::pow(point.y - y - 0.5, 2));
            window.at<cv::Vec3b>(y, x + 1)[2] = static_cast<uchar>(std::max(temp, (1 - distance) * 255));

        }







    }
}

//用遍历来实现贝塞尔曲线
cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
	int n = control_points.size();
    
	if (n == 1)
	{
		return control_points[0];
	}
	std::vector<cv::Point2f> new_points(control_points);
    while (new_points.size()>1)
    {
		std::vector<cv::Point2f> temp_points;
        for (int i = 0; i < new_points.size() - 1; i++)
        {
			auto point = (1 - t) * new_points[i] + t * new_points[i + 1];
			temp_points.push_back(point);

        }

		new_points = temp_points;
    }
	return new_points[0];

}






void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
	for (float t = 0.0; t <= 1.0; t += 0.001)
	{
		auto point = recursive_bezier(control_points, t);
		window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
	}

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            naive_bezier(control_points, window);
              // bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
