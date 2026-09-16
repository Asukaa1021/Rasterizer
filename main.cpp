// Demo entry point: configure the scene, draw a triangle, and display a grayscale image.
#include <iostream>
#include <opencv2/opencv.hpp>
#include "raster_math.h"
#include "Rasterizer.h"
#include "vertex.h"

int main()
{
	const int width = 800;
	const int height = 600;

	Rasterizer rasterizer = Rasterizer(width, height);

	rasterizer.setView(Mat4::lookAt(
		Vec3(0, 0, 5),
		Vec3(0, 0, 0),
		Vec3(0, 1, 0)
	));

	const float pi = acos(-1);
	rasterizer.setProjection(Mat4::perspective(
		pi / 3.0f,
		(float)width / (float)height,
		0.1f,
		100.0f
	));	

	Triangle triangle = Triangle{
		Vertex(Vec3(-1, -1, 3), Vec3(1, 1, 1)),
		Vertex(Vec3(1, -1, 0), Vec3(1, 1, 1)),
		Vertex(Vec3(0, 0, -1), Vec3(1, 1, 1))
	};

	rasterizer.drawTriangle(triangle);

	const std::vector<unsigned char> pixels = rasterizer.get_pixels();
	cv::Mat image(height, width, CV_8UC1);

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			image.at<unsigned char>(y, x) = pixels[x + y * width] ? 255 : 0;
			//image [hang] [lie]
		}
	}


	cv::imshow("ZQYZ's Rasterizer",image);
	cv::waitKey(0);
}