/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#pragma once
#include <vector>
#include "IGlyphCallback.h"

struct CDblPoint
{
	CDblPoint(double x, double y)
	{
		this->x = x;
		this->y = y;
	}
    double x;
	double y;
};

struct IGlyphCallback
{
    virtual void glBegin(int mode) = 0;
    virtual void glVertex2d(double x, double y) = 0;
    virtual void glEnd() = 0;
};

class WidthGlyphCallback : public IGlyphCallback
{
public:
    WidthGlyphCallback()
    {
    }
    void glBegin(int mode)
    {
        _points.push_back(std::vector<CDblPoint>());
    }

    void glVertex2d(double x, double y)
    {
         _points.back().push_back(CDblPoint(x, y));
    }

    void glEnd()
    {
        if (_points.back().size() < 2)
			_points.pop_back();
    }

	double getWidth() const
	{
		if (_points.empty())
			return 0.0;
		double minX = DBL_MAX, maxX = -DBL_MAX;
		for (const auto& points : _points)
		{
			for (const auto& pt : points)
			{
				if (pt.x < minX)
					minX = pt.x;
				if (pt.x > maxX)
					maxX = pt.x;
			}
		}
		return maxX - minX;
	}
private:
    std::vector<std::vector<CDblPoint>> _points;
};
