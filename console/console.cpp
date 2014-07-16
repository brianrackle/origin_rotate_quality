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

/* math */

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

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to func
std::chrono::nanoseconds rotation_iteration(double_t const increment, std::function<void(Coord2D const&)> const& func)
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
std::chrono::nanoseconds polar_iteration(double_t const increment, std::function<void(Coord2D const&)> const& func)
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
	for (int i = 0; i < count; ++i)
		file << "| " << items << " ";
	file << "|\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ofstream outfile("results.markdown");
	//link TOC so we can put a link back at the beginning of each header
	outfile << "<a name = \"toc\"></a>"
		<< "\n\n[TOC]\n\n";

	const double_t increment = 1.0e-2;

	double count;
	double sum;
	double mean;
	double minDelta;
	double maxDelta;

	outfile << "# Vector Rotation\n";

	outfile << "## " << "Dot Results\n";
	table_header(outfile, "lr", "Vector", "Radians");
	outfile << "### Duration: " << rotation_iteration(increment, [&](Coord2D const& vector)
	{ 
		table_row(outfile, 2, cts(vector).c_str(), dts(std::acos(dot_angle(vector))).c_str());
	}).count() << "ns\n\n";

	outfile << "## " << "Trig Results\n";
	table_header(outfile, "lr", "Vector", "Radians");
	outfile << "### Duration: " << rotation_iteration(increment, [&](Coord2D const& vector)
	{ 
		table_row(outfile, 2, cts(vector).c_str(), dts(trig_angle(vector)).c_str());
	}).count() << "ns\n\n";

	outfile << "# Polar Rotation\n";

	outfile << "## " << "Dot Results\n";
	table_header(outfile, "lr", "Vector", "Radians");
	outfile << "### Duration: " << polar_iteration(increment, [&](Coord2D const& vector)
	{
		table_row(outfile, 2, cts(vector).c_str(), dts(std::acos(dot_angle(vector))).c_str());
	}).count() << "ns\n";

	outfile << "## " << "Trig Results\n";
	table_header(outfile, "lr", "Vector", "Radians");
	outfile << "### Duration: " << polar_iteration(increment, [&](Coord2D const& vector)
	{
		table_row(outfile, 2, cts(vector).c_str(), dts(trig_angle(vector)).c_str());
	}).count() << "ns\n";
	outfile << "[back to top](#toc)" << std::endl;
	return EXIT_SUCCESS;
}


