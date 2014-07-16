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

namespace func_def
{
	using angle = std::function < double_t(Coord2D const& vector) >;
	using range = std::function < double_t(double_t const) > ;
	using iteration = std::function < std::chrono::nanoseconds(double_t const, angle const&) >;
}

/* math */

//Calculate the angle of the vector to the horizontal.
 double_t dot_angle(Coord2D const& vector)
{
	 return std::acos(vector.x / std::hypot(vector.x, vector.y));
}

 double_t dot_range(double_t const angle)
 {
	 return angle > M_PI ? (2 * M_PI) - angle : angle;	 
 }

//Calculate the angle of the vector to the horizontal using atan2.
 double_t trig_angle(Coord2D const& line)
{
	return 	std::atan2(line.y, line.x);
}

 double_t trig_range(double_t const angle)
 {
	 return angle > M_PI ? -((2 * M_PI) - angle) : angle;
 }

//Apply rotation matrix to the vector using radians
Coord2D rotate(Coord2D const& vector, double_t const radians)
{
	double_t s = std::sin(radians);
	double_t c = std::cos(radians);

	return { vector.x * c - vector.y * s, vector.x * s + vector.y * c };
}

//increment lhs by rhs
void increment_value(double_t & lhs, double_t const& rhs)
{
	lhs += rhs;
}

/* iteration */

std::chrono::nanoseconds rotation_iteration
(double_t const increment, func_def::angle const& func)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	auto start = time::now();
	for (double_t i = 0; i <= M_PI * 2; increment_value(i, increment))
		func(rotate({ 1, 0 }, i > M_PI ? -((2 * M_PI) - i) : i));

	return time::now() - start;
};

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to func
std::chrono::nanoseconds rotation_inc_iteration
(double_t const increment, func_def::angle const& func)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	Coord2D vector = { 1, 0 };
	auto start = time::now();
	for (double_t i = 0; i <= M_PI * 2; increment_value(i, increment))
	{
		func(vector);
		vector = rotate(vector, increment);
	}
	return time::now() - start;
};

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to func
std::chrono::nanoseconds polar_iteration
(double_t const increment, func_def::angle const& func)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	auto start = time::now();
	for (double_t i = 0; i <= M_PI * 2; increment_value(i, increment))
		func({ std::cos(i), std::sin(i) });
	return time::now() - start;
};

//complex number rotation

/* string conversion */

//convert double to string
 std::string dts(double_t d)
{
	char buf[50];
	sprintf_s(buf, "%.16f", d);
	return buf;
}

//convert Coord2d to string
 std::string cts(Coord2D const& vector)
{
	return dts(vector.x) + " , " + dts(vector.y);
}

void table_header(std::ofstream & file, char const* format)
{
	file << "|\n";

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

//create markdown table header
template<typename ... Types>
void table_header(std::ofstream & file, char const* format, char const * first, Types ... rest)
{
	file << "| " << first << " ";
	table_header(file, format, rest...);
}

void table_row(std::ofstream & file)
{
	file << "|\n";
}

//create markdown table row
template <typename ... Types>
void table_row(std::ofstream & file, char const* first, Types ... rest)
{
	file << "| " << first << " ";
	table_row(file, rest...);
}

//create markdown heading
void heading(std::ofstream & file, int const level, char const* head)
{
	for (int i = 0; i < level; ++i)
		file << "#";
	file << " " << head << "\n";
}


//output angles and stats using iteration functon
void calculation_block
(std::ofstream & outfile, double_t const increment, func_def::iteration const& itrn, func_def::angle const& calcAngle, func_def::range const& calcRange)
{
	size_t count = 0;
	double_t sumDelta = 0;
	double_t minDelta = std::numeric_limits<double_t>::max();
	double_t maxDelta = std::numeric_limits<double_t>::lowest();

	auto duration = itrn(increment, [&](Coord2D const& vector) -> double_t
	{
		double_t angle = calcAngle(vector);
		double_t trueAngle = calcRange(increment * count);
		
		double_t delta = std::abs(trueAngle - angle);
		if (count != 0)
		{
			sumDelta += delta;
			if (minDelta > delta)
				minDelta = delta;
			if (maxDelta < delta)
				maxDelta = delta;
		}

		table_row(outfile,
			std::to_string(count).c_str(),
			cts(vector).c_str(),
			dts(angle).c_str(),
			dts(trueAngle).c_str(),
			dts(delta).c_str());


		++count;
		return angle;
	}).count();

	heading(outfile, 3, ("Duration: " + std::to_string(duration) + " ns").c_str());
	heading(outfile, 3, ("Mean Delta: " + dts(sumDelta / count)).c_str());
	heading(outfile, 3, ("Min Delta: " + dts(minDelta)).c_str());
	heading(outfile, 3, ("Max Delta: " + dts(maxDelta)).c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ofstream outfile("results.markdown");
	
	outfile << "<a name = \"toc\"></a>"
		<< "\n\n[TOC]\n\n";

	const double_t increment = 1.0e-2;

	heading(outfile, 1, "Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, rotation_iteration, dot_angle, dot_range);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, rotation_iteration, trig_angle, trig_range);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 1, "Incremental Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, rotation_inc_iteration, dot_angle, dot_range);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, rotation_inc_iteration, trig_angle, trig_range);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 1, "Polar Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, polar_iteration, dot_angle, dot_range);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Angle", "True Angle", "Delta");
	calculation_block(outfile, increment, polar_iteration, trig_angle, trig_range);
	outfile << "[back to top](#toc)" << "\n\n";

	return EXIT_SUCCESS;
}


