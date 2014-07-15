// console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <cstdlib>
#include <cmath>
#include <vector>
#include <chrono>
#include <functional>
#include <fstream>
#include <cstdarg>

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

__forceinline std::string dts(double_t d)
{
	char buf[50];
	sprintf_s(buf, "%.16f", d);
	return buf;
}

__forceinline std::string cts(Coord2D const& vector)
{
	return dts(vector.x) + " , " + dts(vector.y);
}

__forceinline void TableHeader(std::ofstream & file, char const* format, char const* header...)
{
	va_list args;
	va_start(args, format);
	for (char const* pos = format; *pos; ++pos)
		file << "| " << va_arg(args, char const*) << " ";
	file << "|\n";
	va_end(args);

	for (char const* pos = format; *pos; ++pos)
		switch (*pos)
		{
		case 'l':
			file << "|:--- ";
			break;
		case 'r':
			file << "| ---:";
			break;
		case 'c':
			file << "|:---:";
			break;
		default:
			break;
		}
	file << "|\n";
}

#define RADIAN_FRACTION 1.0e-2

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

	std::ofstream outfile("results.markdown");
	//link TOC so we can put a link back at the beginning of each header
	outfile << "[TOC]\n";
	
	auto tableHeader = [](std::ofstream & file, char const* tableName)
	{
		file << "## " << tableName << " Results\n"
			<< "| Vector | Radians |\n"
			<< "|:------ | -------:|\n";
	};

	//rotate because error builds up in vector.
	//vs
	//vector being recalculated from polar on each iteration
	{
		double count;
		double sum;
		double mean;
		double minDelta;
		double maxDelta;

		//could also store delta of every-1 angle and make a 1d heatmap out of it

		auto rotationIterator =
			[](std::function<void(Coord2D const&)> func)
			-> std::chrono::nanoseconds
		{
			using time = std::chrono::high_resolution_clock;
			std::chrono::nanoseconds duration;

			Coord2D vector = { 1, 0 };
			auto start = time::now();
			for (double_t i = 0; i <= M_PI * 2 + RADIAN_FRACTION; Increment(i, RADIAN_FRACTION))
			{
				func(vector);
				Rotate(vector, RADIAN_FRACTION);
			}
			return time::now() - start;
		};

		outfile << "# Vector Rotation\n";

		outfile << "## " << "Dot Results\n";
		TableHeader(outfile, "lr", "Vector", "Radians");
		outfile << "### Duration: " << rotationIterator([&](Coord2D const& vector)
		{ 
			outfile << "|"
				<< " " << cts(vector).c_str() << " |"
				<< " " << dts(std::acos(DotAngleH(vector))).c_str() << " |\n";
		}).count() << "ns\n\n";

		tableHeader(outfile, "Trig");
		outfile << "### Duration: " << rotationIterator([&](Coord2D const& vector)
		{ 
			outfile << "|"
				<< " " << cts(vector).c_str() << " |"
				<< " " << dts(TrigAngleH(vector)).c_str() << " |\n";
		}).count() << "ns\n\n";
	}

	{
		auto polarIterator =
			[](std::function<void(Coord2D const&)> func)
			-> std::chrono::nanoseconds
		{
			using time = std::chrono::high_resolution_clock;
			std::chrono::nanoseconds duration;

			auto start = time::now();
			for (double_t i = 0; i <= (M_PI * 2) + RADIAN_FRACTION; Increment(i, RADIAN_FRACTION))
			{
				func({ std::cos(i), std::sin(i) });
			}
			return time::now() - start;
		};

		outfile << "# Polar Rotation\n";

		tableHeader(outfile, "Dot");
		outfile << "### Duration: " << polarIterator([&](Coord2D const& vector)
		{
			outfile << "|"
				<< " " << cts(vector).c_str() << " |"
				<< " " << dts(std::acos(DotAngleH(vector))).c_str() << " |\n";
		}).count() << "ns\n";

		tableHeader(outfile, "Trig");
		outfile << "### Duration: " << polarIterator([&](Coord2D const& vector)
		{
			outfile << "|"
				<< " " << cts(vector).c_str() << " |"
				<< " " << dts(TrigAngleH(vector)).c_str() << " |\n";
		}).count() << "ns\n";
	}

	//verify against http://products.wolframalpha.com/api/
	//can use this? http://wolframalphaapi20.codeplex.com/
	//verify against: http://www.wolframalpha.com/input/?i=atan%280%2C-1%29
	return EXIT_SUCCESS;
}


