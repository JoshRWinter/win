#pragma once

#include <cmath>

namespace win
{

template <typename T> T distance(T x1, T y1, T x2, T y2)
{
	return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

template <typename T> static float normalize_angle(T a)
{
	const T PI2 = 2.0f * M_PI;

	while (a > PI2)
		a -= PI2;

	while (a < 0.0f)
		a += PI2;

	return a;
}

template <typename T> T angle_align(T angle, T target, const T step)
{
	target = normalize_angle(target);
	angle = normalize_angle(angle);

	if (target > angle)
	{
		if (target - angle > M_PI)
		{
			angle -= step;

			if (angle < 0.0f)
			{
				angle += 2.0f * M_PI;

				if (angle < target)
					angle = target;
			}
		} else
		{
			angle += step;

			if (angle > target)
				angle = target;
		}
	} else
	{
		if (angle - target > M_PI)
		{
			angle += step;

			if (angle > 2.0f * M_PI)
			{
				angle -= 2.0f * M_PI;

				if (angle > target)
					angle = target;
			}
		} else
		{
			angle -= step;

			if (angle < target)
				angle = target;
		}
	}

	return angle;
}

template <typename T> T angle_diff(T a, T b)
{
	a = normalize(a);
	b = normalize(b);

	const float diff = fabs(a - b);
	if (diff > M_PI)
		return diff - M_PI;

	return diff;
}

template <typename T> T target(T f, T target, T approach)
{
	if (f > target)
	{
		f -= approach;
		if (f < target)
			f = target;
	} else if (f < target)
	{
		f += approach;
		if (f > target)
			f = target;
	}

	return f;
}

template <typename T> T zero(T f, T approach)
{
	return target<T>(f, 0.0, approach);
}

}
