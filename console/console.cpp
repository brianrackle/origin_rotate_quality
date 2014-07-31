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
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

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
 std::string to_string(double_t d)
{
	char buf[50];
	sprintf_s(buf, "%.16f", d);
	return std::string(buf);
}

void append_attribute(rapidxml::xml_document<> &doc, rapidxml::xml_node<> * node, std::pair<std::string const, std::string const>  const& attribute)
{
	node->append_attribute(doc.allocate_attribute(doc.allocate_string(attribute.first.c_str()), doc.allocate_string(attribute.second.c_str())));
}

template <typename ... Types>
void append_attribute(rapidxml::xml_document<> &doc, rapidxml::xml_node<> * node, std::pair<std::string const, std::string const>  const& attribute, Types const& ... attributes)
{
	append_attribute(doc, node, attribute);
	append_attribute(doc, node, attributes...);
}

rapidxml::xml_node<> * append_node(rapidxml::xml_document<> &doc, char const* node_name)
{
	return doc.allocate_node(rapidxml::node_element, doc.allocate_string(node_name));
}

template <typename ... Types>
rapidxml::xml_node<> * append_node(rapidxml::xml_document<> &doc, char const* node_name, std::pair<std::string const, std::string const>  const& attribute, Types const& ... attributes)
{
	rapidxml::xml_node<> *node = append_node(doc, doc.allocate_string(node_name));
	append_attribute(doc, node, attribute, attributes...);
	return node;
}

template <typename T>
void calculation_block
(T & out, rapidxml::xml_node<> * parent, double_t const increment, func_def::iterate const& iterate, func_def::theta const& calc_theta, func_def::clamp const& clamp)
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
		std::string x = to_string(vector.x);
		std::string y = to_string(vector.y);
		//scale to output
		parent->append_node(
			append_node(out, "line",
			std::make_pair("x1", "0.0"),
			std::make_pair("y1", "0.0"),
			std::make_pair("x2", x),
			std::make_pair("y2", y),
			std::make_pair("style", "stroke-width:1;" + 
			("stroke:rgb(" + std::to_string(255) + ",0,0)"))
			));
		//every 255 move left

		x.clear();
		y.clear();
		++count;
	}).count();
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ofstream outfile("results.svg");
	rapidxml::xml_document<> doc;

	rapidxml::xml_node<> * svg = append_node(doc, "svg", std::make_pair("att0", "value0"), std::make_pair("att1", "value1"));
	doc.append_node(svg);

	const double_t increment = 1.0e-2;

	calculation_block(doc, svg, increment, polar_iteration, dot_theta, dot_clamp);
	calculation_block(doc, svg, increment, polar_iteration, trig_theta, trig_clamp);

	outfile << doc;

	return EXIT_SUCCESS;
}