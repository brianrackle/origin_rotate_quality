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

struct coord2d
{
	double_t x;
	double_t y;
};

namespace func_def
{
	using theta = std::function < double_t(coord2d const&) >;
	using clamp = std::function < double_t(double_t const) >;
	using iterable = std::function < void(coord2d const&, double_t const) >;
	using iterate = std::function < std::chrono::nanoseconds(double_t const, iterable const&) >;
}

/* math */

//Calculate the theta of the vector to the horizontal.
 double_t dot_theta(coord2d const& vector)
{
	 return std::acos(vector.x / std::hypot(vector.x, vector.y));
}

 double_t dot_clamp(double_t const theta)
 {
	 return theta > M_PI ? (2 * M_PI) - theta : theta;	 
 }

//Calculate the theta of the vector to the horizontal using atan2.
 double_t trig_theta(coord2d const& line)
{
	return 	std::atan2(line.y, line.x);
}

 double_t trig_clamp(double_t const theta)
 {
	 return theta > M_PI ? -((2 * M_PI) - theta) : theta;
 }

//Apply rotation matrix to the vector using theta
coord2d rotate(coord2d const& vector, double_t const theta)
{
	double_t s = std::sin(theta);
	double_t c = std::cos(theta);

	return { vector.x * c - vector.y * s, vector.x * s + vector.y * c };
}

/* iterate */

std::chrono::nanoseconds rotation_iteration
(double_t const increment, func_def::iterable const& iterable)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	auto start = time::now();
	double_t theta = 0;
	for (size_t i = 0; (theta = i*increment) <= M_PI * 2; ++i)
		iterable(rotate({ 1, 0 }, theta), theta);
	return time::now() - start;
};

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to iterable
std::chrono::nanoseconds rotation_inc_iteration
(double_t const increment, func_def::iterable const& iterable)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	coord2d vector = { 1, 0 };
	auto start = time::now();
	double_t theta = 0;
	for (size_t i = 0; (theta = i*increment) <= M_PI * 2; ++i)
	{
		iterable(vector, theta);
		vector = rotate(vector, increment);
	}
	return time::now() - start;
};

//iterate through all the values between 0 and M_PI * 2 at increment intervals
//at each interval create a vector at the interval value and pass the vector to iterable
std::chrono::nanoseconds polar_iteration
(double_t const increment, func_def::iterable const& iterable)
{
	using time = std::chrono::high_resolution_clock;
	std::chrono::nanoseconds duration;

	auto start = time::now();
	double_t theta = 0;
	for (size_t i = 0; (theta = i*increment) <= M_PI * 2; ++i)
		iterable({ std::cos(theta), std::sin(theta) }, theta);
	return time::now() - start;
};

/* string conversion */

//convert double to string
 std::string dts(double_t d)
{
	char buf[50];
	sprintf_s(buf, "%.16f", d);
	return buf;
}

//convert coord2d to string
 std::string cts(coord2d const& vector)
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
//pass the theta to the func_def::iterate function so that the theta used to calculate the vector can be used for comparison
//output thetas and stats using iterate functon
void calculation_block
(std::ofstream & outfile, double_t const increment, func_def::iterate const& iterate, func_def::theta const& calc_theta, func_def::clamp const& clamp)
{
	size_t count = 0;
	double_t sumDelta = 0;
	double_t minDelta = std::numeric_limits<double_t>::max();
	double_t maxDelta = std::numeric_limits<double_t>::lowest();

	auto duration = iterate(increment, [&](coord2d const& vector, double_t const theta)
	{
		double_t calced_theta = calc_theta(vector);
		double_t clamped_theta = clamp(theta);
		
		double_t delta = std::abs(clamped_theta - calced_theta);
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
			dts(calced_theta).c_str(),
			dts(clamped_theta).c_str(),
			dts(delta).c_str());

		++count;
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
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, rotation_iteration, dot_theta, dot_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, rotation_iteration, trig_theta, trig_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 1, "Incremental Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, rotation_inc_iteration, dot_theta, dot_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, rotation_inc_iteration, trig_theta, trig_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 1, "Polar Rotation");
	heading(outfile, 2, "Dot Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, polar_iteration, dot_theta, dot_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	heading(outfile, 2, "Trig Results");
	table_header(outfile, "lcrrr", "Count", "Vector", "Calced Theta", "True Theta", "Delta");
	calculation_block(outfile, increment, polar_iteration, trig_theta, trig_clamp);
	outfile << "[back to top](#toc)" << "\n\n";

	return EXIT_SUCCESS;
}


