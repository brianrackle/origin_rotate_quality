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
#include <string>

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

/* math */
using angle = std::function<double_t(Coord2D const& vector)>;
//Calculate the angle of the vector to the horizontal.
__forceinline double_t dot_angle(Coord2D const& vector)
{
	return vector.x / std::hypot(vector.x, vector.y);
}

//Calculate the angle of the vector to the horizontal using atan2.
__forceinline double_t trig_angle(Coord2D const& line)
{
	return 	std::atan2(line.y, line.x);
}

//Apply rotation matrix to the vector using radians
__forceinline void rotate(Coord2D & vector, double_t radians)
{
	double_t s = std::sin(radians);
	double_t c = std::cos(radians);

	vector = { vector.x * c - vector.y * s, vector.x * s + vector.y * c };
}

//increment lhs by rhs
__forceinline void increment_value(double_t & lhs, double_t const& rhs)
{
	lhs += rhs;
}

/* iteration */
using iteration =
std::function<std::chrono::nanoseconds(double_t const, angle const&)>;

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to func
std::chrono::nanoseconds rotation_iteration
(double_t const increment, std::function<void(Coord2D const&)> const& func)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	Coord2D vector = { 1, 0 };
	auto start = time::now();
	for (double_t i = 0; i <= M_PI * 2; increment_value(i, increment))
	{
		func(vector);
		rotate(vector, increment);
	}
	return time::now() - start;
};

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to func
std::chrono::nanoseconds polar_iteration
(double_t const increment, std::function<void(Coord2D const&)> const& func)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	auto start = time::now();
	for (double_t i = 0; i <= M_PI * 2; increment_value(i, increment))
		func({ std::cos(i), std::sin(i) });
	return time::now() - start;
};

/* string conversion */

//convert double to string
__forceinline std::string dts(double_t d)
{
	char buf[50];
	sprintf_s(buf, "%.16f", d);
	return buf;
}

//convert Coord2d to string
__forceinline std::string cts(Coord2D const& vector)
{
	return dts(vector.x) + " , " + dts(vector.y);
}

//create markdown table header
__forceinline void table_header(std::ofstream & file, char const* format, char const* headers...)
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

//create markdown table row
__forceinline void table_row(std::ofstream & file, int const count, char const* items...)
{
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; ++i)
		file << "| " << va_arg(args, char const*) << " ";
	file << "|\n";
	va_end(args);
}

//create markdown heading
__forceinline void heading(std::ofstream & file, int const level, char const* head)
{
	for (int i = 0; i < level; ++i)
		file << "#";
	file << " " << head << "\n";
}


//output angles and stats using iteration functon
__forceinline void calculation_block
(std::ofstream & outfile, double_t const increment, iteration const& itrn, angle const& itrbl)
{
	double_t count = 0;
	double_t sum = 0;
	double_t minDelta = std::numeric_limits<double_t>::max();
	double_t maxDelta = std::numeric_limits<double_t>::lowest();
	double_t trueAngle = 0;

	auto duration = itrn(increment, [&](Coord2D const& vector)
	{
		trueAngle += increment;
		double_t angle = itrbl(vector);
		sum += angle;

		if (count != 0)
		{
			double_t delta = trueAngle - angle;
			if (minDelta > delta)
				minDelta = delta;
			if (maxDelta < delta)
				maxDelta = delta;
		}
		table_row(outfile, 4,
			std::to_string(count).c_str(),
			cts(vector).c_str(),
			dts(angle).c_str(),
			dts(trueAngle).c_str());

		++count;
	}).count();
	heading(outfile, 3, ("Duration: " + std::to_string(duration) + " ns").c_str());
	heading(outfile, 3, ("Mean: " + dts(sum / count)).c_str());
	heading(outfile, 3, ("Min Delta: " + dts(minDelta)).c_str());
	heading(outfile, 3, ("Max Delta: " + dts(maxDelta)).c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ofstream outfile("results.markdown");
	
	outfile << "<a name = \"toc\"></a>"
		<< "\n\n[TOC]\n\n";

	const double_t increment = 1.0e-2;

	heading(outfile, 1, "Vector Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lr", "Vector", "Radians");
	calculation_block(outfile, increment,
		rotation_iteration, [](Coord2D const& vector){ return std::acos(dot_angle(vector)); });
	outfile << "[back to top](#toc)" << "\n\n";

	//heading(outfile, 2, "Trig Results");
	//table_header(outfile, "lr", "Vector", "Radians");
	//calculation_block(outfile, increment,
	//	rotation_iteration(increment, [&](Coord2D const& vector)
	//{ 
	//	table_row(outfile, 2, cts(vector).c_str(), 
	//		dts(trig_angle(vector)).c_str());
	//}));
	//outfile << "[back to top](#toc)" << "\n\n";

	//heading(outfile, 1, "Polar Rotation");
	//heading(outfile, 2, "Dot Results");
	//table_header(outfile, "lr", "Vector", "Radians");
	//calculation_block(outfile, increment,
	//	polar_iteration(increment, [&](Coord2D const& vector)
	//{
	//	table_row(outfile, 2, cts(vector).c_str(), 
	//		dts(std::acos(dot_angle(vector))).c_str());
	//}));
	//outfile << "[back to top](#toc)" << "\n\n";

	//heading(outfile, 2, "Trig Results");
	//table_header(outfile, "lr", "Vector", "Radians");
	//calculation_block(outfile, increment,
	//	polar_iteration(increment, [&](Coord2D const& vector)
	//{
	//	table_row(outfile, 2, cts(vector).c_str(),
	//		dts(trig_angle(vector)).c_str());
	//}));
	//outfile << "[back to top](#toc)" << "\n\n";

	return EXIT_SUCCESS;
}


