#pragma once

#include <vector>
#include <math.h>

#define M_PI  3.14159268f
#define SPEED_OF_SOUND 343.0f

//==============================================================================
struct Point3D
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

//==============================================================================
class Math3D
{
public:
	static Point3D substruct(const Point3D& a, const Point3D& b)
	{
		Point3D tmp;
		tmp.x = a.x - b.x;
		tmp.y = a.y - b.y;
		tmp.z = a.z - b.z;

		return tmp;
	}

	static Point3D devide(const Point3D& a, float b)
	{
		Point3D tmp;
		tmp.x = a.x / b;
		tmp.y = a.y / b;
		tmp.z = a.z / b;

		return tmp;
	}

	static Point3D sum(const Point3D& a, const Point3D& b)
	{
		Point3D tmp;
		tmp.x = a.x + b.x;
		tmp.y = a.y + b.y;
		tmp.z = a.z + b.z;

		return tmp;
	}

	static Point3D multiply(const Point3D& a, float b)
	{
		Point3D tmp;
		tmp.x = a.x * b;
		tmp.y = a.y * b;
		tmp.z = a.z * b;

		return tmp;
	}

	static float lenght(const Point3D& a)
	{
		return std::sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
	}

	static Point3D normalize(const Point3D& a)
	{
		return devide(a, lenght(a));
	}

	//==============================================================================
	static std::vector<Point3D> CreateFibonacciSphere(int numPoints)
	{
		std::vector<Point3D> points;
		points.reserve(numPoints);

		// Calculate the golden angle in radians
		const float goldenAngle = M_PI * (3.0f - std::sqrt(5.0f));

		for (int i = 0; i < numPoints; ++i) {
			// Calculate the y coordinate, ranging from -1 to 1
			float y = 1.0f - (2.0f * i) / (numPoints - 1); // linearly spaced from 1 to -1
			float radius = std::sqrt(1.0f - y * y); // radius of the circle at height y

			// Azimuthal angle around the y-axis
			float theta = i * goldenAngle;

			// Convert spherical coordinates to Cartesian coordinates
			float x = radius * std::cos(theta);
			float z = radius * std::sin(theta);

			points.push_back({ x, y, z });
		}

		return points;
	}

	//==============================================================================
	// Get point on the box
	static Point3D GetIntersection(Point3D outerPoint, Point3D insidePoint, Point3D mins, Point3D maxs)
	{
		Point3D d = normalize(substruct(outerPoint, insidePoint));

		float txmax = (outerPoint.x - maxs.x) / d.x;
		float txmin = (outerPoint.x - mins.x) / d.x;

		float tymax = (outerPoint.y - maxs.y) / d.y;
		float tymin = (outerPoint.y - mins.y) / d.y;

		float tzmax = (outerPoint.z - maxs.z) / d.z;
		float tzmin = (outerPoint.z - mins.z) / d.z;

		float tx = std::fmin(txmax, txmin);
		float ty = std::fmin(tymax, tymin);
		float tz = std::fmin(tzmax, tzmin);

		float t = std::fmax(tx, std::fmax(ty, tz));

		return substruct(outerPoint, multiply(d, t));
	}

	//==============================================================================
	// Get distances to intersections between box and rays from Fibonacci sphere
	static void GetDistances(std::vector<float>& distances, const float lenght, const float width, const float height, Point3D spherePosition, int rayCount)
	{
		Point3D mins, maxs;
		mins.x = -0.5f * lenght;
		mins.y = -0.5f * width;
		mins.z = -0.5f * height;

		maxs.x = 0.5f * lenght;
		maxs.y = 0.5f * width;
		maxs.z = 0.5f * height;

		std::vector<Point3D> points = Math3D::CreateFibonacciSphere(rayCount);

		// TODO: Remove this hack
		for (int i = 0; i < rayCount; i++)
		{
			points[i].x = points[i].x * 100.0f;
			points[i].y = points[i].y * 100.0f;
			points[i].z = points[i].z * 100.0f;
		}

		for (int i = 0; i < rayCount; i++)
		{
			Point3D p = Math3D::GetIntersection(points[i], spherePosition, mins, maxs);
			distances[i] = Math3D::lenght(p);
		}
	}
};