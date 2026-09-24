//C:\Users\lenovo\Desktop\图形学\Rasterizer\models\elephant.obj 大象
//C:\Users\lenovo\Desktop\图形学\Rasterizer\models\sphere.obj 球体
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "raster_math.h"
#include "Rasterizer.h"
#include "vertex.h"
#include "Mesh.h"
#include <algorithm>
#include "obj_loader.h"
#include <random>
#include "mesh_simplifier.h"

struct CameraState
{
	Vec3 target;
	float distance = 5.0f;
	float yaw = 0.0f;
	float pitch = 0.0f;
	bool dragging = false;
	int lastX = 0;
	int lastY = 0;
};

Vec3 calculateEye(const CameraState* camera)
{
	float horizontal = camera -> distance * std::cos(camera -> pitch);

	float x = horizontal * std::sin(camera -> yaw);
	float y = camera->distance * std::sin(camera->pitch);
	float z = horizontal * std::cos(camera -> yaw);

	return Vec3(x, y, z) + camera -> target;
}

Mat4 makeFitTransform(const Mesh& mesh)
{
	float max_x = std::numeric_limits<float>::lowest(), max_y = std::numeric_limits<float>::lowest(), max_z = std::numeric_limits<float>::lowest(),
		   min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max(), min_z = std::numeric_limits<float>::max();

	for (Vec3 position : mesh.positions)
	{
		max_x = std::max(max_x, position.get_x());
		max_y = std::max(max_y, position.get_y());
		max_z = std::max(max_z, position.get_z());
		min_x = std::min(min_x, position.get_x());
		min_y = std::min(min_y, position.get_y());
		min_z = std::min(min_z, position.get_z());
	}
	float centerX = (max_x + min_x) * 0.5f;
	float centerY = (max_y + min_y) * 0.5f;
	float centerZ = (max_z + min_z) * 0.5f;

	Mat4 translation = Mat4::translation(
		-centerX, -centerY, -centerZ
	);
	
	float longest = std::max({ max_x - min_x,max_y - min_y,max_z - min_z });
	if (longest == 0)
	{
		return translation;
	}
	float scale = 2.0f / longest;
	Mat4 scaling = Mat4::scaling(scale,scale,scale);

	return scaling * translation;
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	CameraState* camera = static_cast<CameraState*> (userdata);

	
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		camera -> dragging = true;
		camera -> lastX = x;
		camera -> lastY = y;
	}
	else if (event == cv::EVENT_LBUTTONUP)
	{
		camera -> dragging = false;
	}
	else if (event == cv::EVENT_MOUSEMOVE && camera -> dragging)
	{
		const float sensitivity = 0.005f;
		float dx = x - camera->lastX;
		float dy = y - camera->lastY;

		camera->yaw -= dx * sensitivity;
		camera->pitch += dy * sensitivity;

		camera->pitch = std::clamp(
			camera->pitch,
			-1.553343f,
			1.553343f
		);

		camera->lastX = x;
		camera->lastY = y;
	}
	
	if (event == cv::EVENT_MOUSEWHEEL)
	{
		int delta = cv::getMouseWheelDelta(flags);

		if (delta > 0)
		{
			camera -> distance *= 0.9f;
		}
		else if (delta < 0)
		{
			camera -> distance /= 0.9f;
		}

		camera->distance = std::clamp(camera->distance, 2.0f, 20.0f);
	}
}


std::size_t selectLod(float distance,std::size_t currentLOD)
{
	if (currentLOD == 0)
	{
		if (distance <= 6.5) return 0;
		else if (distance <= 12) return 1;
		else return 2;
	}
	else if (currentLOD == 1)
	{
		if (distance <= 5.5f) return 0;
		else if (distance <= 12) return 1;
		else return 2;
	}
	else if(currentLOD == 2)
	{
		if (distance <= 6) return 0;
		else if (distance <= 11.5) return 1;
		else return 2;
	}

	return 0;
}

int main()
{
	const int width = 800;
	const int height = 600;

	Rasterizer rasterizer = Rasterizer(width, height);
	Vec3 eye = Vec3(0, 0, 5);

	Mesh mesh;
	std::string path;

	std::cout << "请输入路径：" << std::endl;
	std::getline(std::cin, path);

	if (!loadOBJ(path, mesh)) return 1;

	if (mesh.positions.size() == 0 || mesh.triangles.size() == 0)
	{
		return 1;
	}

	rasterizer.setModel(makeFitTransform(mesh));

	const float pi = acos(-1);
	rasterizer.setProjection(Mat4::perspective(
		pi / 3.0f,
		(float)width / (float)height,
		0.1f,
		100.0f
	));	
	
	const Vec3& lightDir = Vec3(0, 0, 1);
	const Vec3& gray = Vec3(0.7f, 0.7f, 0.7f);

	const std::vector<Vec3>& pixels = rasterizer.get_pixels();
	cv::Mat image(height, width, CV_8UC3);
	
	CameraState camera;

	cv::namedWindow("ZQYZ's Rasterizer");
	cv::setMouseCallback(
		"ZQYZ's Rasterizer",
		onMouse,
		&camera
	);

	std::size_t targetTriangleCount = mesh.triangles.size() / 2;
	Mesh simplifiedMesh = SimplifyMesh(mesh, targetTriangleCount);

	std::vector<Mesh> lodMeshes;
	
	lodMeshes.push_back(mesh);
	lodMeshes.push_back(simplifiedMesh);
	lodMeshes.push_back(
		SimplifyMesh(mesh, mesh.triangles.size() / 4)
	);

	for (int i = 0; i < lodMeshes.size(); i++)
	{
		std::cout << "Lod：" << i << std::endl;
		std::cout << "简化后面数：" << lodMeshes[i].triangles.size() << std::endl;
	}

	std::size_t currentLOD = 0;
	while (true)
	{
		rasterizer.clear();
		eye = calculateEye(&camera);

		rasterizer.setView(Mat4::lookAt(
			eye,
			camera.target,
			Vec3(0, 1, 0)
		));

		std::size_t nextLOD = selectLod(camera.distance,currentLOD);
		

		if (nextLOD != currentLOD)
		{
			std::cout << "当前LOD：" << nextLOD << std::endl;
			std::cout << "当前面数：" << lodMeshes[nextLOD].triangles.size() << std::endl;
			currentLOD = nextLOD;
		}


		const Mesh& displayedMesh = lodMeshes[currentLOD];
		for (int i = 0; i < displayedMesh.triangles.size(); i++)
		{
			Triangle triangle = Triangle{
				Vertex(displayedMesh.positions[displayedMesh.triangles[i][0]],gray),
				Vertex(displayedMesh.positions[displayedMesh.triangles[i][1]],gray),
				Vertex(displayedMesh.positions[displayedMesh.triangles[i][2]],gray)
			};

			rasterizer.drawTriangle(triangle, lightDir, eye);
		}

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				const Vec3& color = pixels[x + y * width];
				image.at<cv::Vec3b>(y, x) = cv::Vec3b(
					cv::saturate_cast<unsigned char>(color.get_z() * 255.0f),
					cv::saturate_cast<unsigned char>(color.get_y() * 255.0f),
					cv::saturate_cast<unsigned char>(color.get_x() * 255.0f)
				);

				//image [hang] [lie]
			}
		}

		cv::imshow("ZQYZ's Rasterizer", image);

		int key = cv::waitKey(1);

		if (key == 27) break;
	}

}