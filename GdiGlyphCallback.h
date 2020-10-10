#pragma once
#include <vector>
#include "IGlyphCallback.h"
#include <Windows.h>

class GdiGlyphCallback : public IGlyphCallback
{
public:
    GdiGlyphCallback(HDC hdc/*, double bottom*/)
    {
        _hdc = hdc;
        //_bottom = bottom;
    }
    void glBegin(int mode)
    {
        //assert(mode == GL_LINE_STRIP);
        _points.clear();
    }

    void glVertex2d(double x, double y)
    {
        // mirror at _bottom so that text isnt upside down.
        //y = _bottom - (y - _bottom);
        _points.push_back({ int(x), int(y) });
    }

    void glEnd()
    {
        if (_points.size() > 1)
            Polyline(_hdc, _points.data(), _points.size());
    }
private:
    HDC _hdc;
    //double _bottom;
    std::vector<POINT> _points;
};
