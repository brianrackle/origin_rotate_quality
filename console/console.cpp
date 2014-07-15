// console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <cstdlib>
#include <cmath>
#include <vector>
#include <chrono>
#include <functional>
#include <iostream>

struct Coord2D
{
	double_t x;
	double_t y;
};

struct Line
{
	Coord2D to;
	Coord2D from;
};

namespace utility
{
	//Calculate the angle of the vector to the horizontal.
	__forceinline double_t DotAngleH(Coord2D const& vector)
	{
		return vector.x / std::hypot(vector.x, vector.y);
	}

	//Calculate the angle of the vector to the horizontal using atan2.
	__forceinline double_t TrigAngleH(Coord2D const& line)
	{
		return 	std::atan2(line.y, line.x);
	}

	//Apply rotation matrix to the vector using radians
	__forceinline void Rotate(Coord2D & vector, double_t radians)
	{
		double_t s = std::sin(radians);
		double_t c = std::cos(radians);

		vector = { vector.x * c - vector.y * s, vector.x * s + vector.y * c };
	}

	//Increment lhs by rhs
	__forceinline void Increment(double_t & lhs, double_t const& rhs)
	{
		lhs += rhs;
	}
}
#define RADIAN_FRACTION 1.0e-8

int _tmain(int argc, _TCHAR* argv[])
{
	//Blog: how to get unit vectors rotated around the origin
	// look at largest angle increment
	// mean angle increment
	// average distance of vector from unit length

	//Example 1: Rotating vector with lambda iterator
	//benefits:
	//	-versatile, can pass any worker into angleIterator
	//	-no memory storage
	//alternatives:
	//	-populate an std::vector with the values and then iterate. Excessive memory usage
	//http://stackoverflow.com/questions/3162643/proper-trigonometry-for-rotating-a-point-around-the-origin

	//THIS WAY SUCKS because error builds up in vector.
	//vs
	//vector being recalculated each iteration
	{
		auto rotationIterator =
			[](std::function<void(Coord2D const&)> func)
			-> std::chrono::nanoseconds
		{
			using time = std::chrono::high_resolution_clock;
			std::chrono::nanoseconds duration;

			Coord2D vector = { 1, 0 };
			auto start = time::now();
			for (double_t i = 0; i <= M_PI * 2 + RADIAN_FRACTION; utility::Increment(i, RADIAN_FRACTION))
			{
				func(vector);
				utility::Rotate(vector, RADIAN_FRACTION);
			}
			return time::now() - start;
		};

		std::cout << "Dot Duration: " << rotationIterator([&](Coord2D const& vector)
		{ utility::DotAngleH(vector); }).count() << "ns\n";
		std::cout << "Trig Duration: " << rotationIterator([&](Coord2D const& vector)
		{ utility::TrigAngleH(vector); }).count() << "ns\n";
	}

	{
		auto polarIterator =
			[](std::function<void(Coord2D const&)> func)
			-> std::chrono::nanoseconds
		{
			using time = std::chrono::high_resolution_clock;
			std::chrono::nanoseconds duration;

			auto start = time::now();
			for (double_t i = 0; i <= (M_PI * 2) + RADIAN_FRACTION; utility::Increment(i, RADIAN_FRACTION))
			{
				func({ std::cos(i), std::sin(i) });
			}
			return time::now() - start;
		};

		std::cout << "Dot Duration: " << polarIterator([&](Coord2D const& vector)
		{ utility::DotAngleH(vector); }).count() << "ns\n";
		std::cout << "Trig Duration: " << polarIterator([&](Coord2D const& vector)
		{ utility::TrigAngleH(vector); }).count() << "ns\n";
	}

	char c;
	std::cin >> c;
	//verify against http://products.wolframalpha.com/api/
	//can use this? http://wolframalphaapi20.codeplex.com/
	//verify against: http://www.wolframalpha.com/input/?i=atan%280%2C-1%29
	return EXIT_SUCCESS;
}


