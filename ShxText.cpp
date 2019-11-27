/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include "ShxText.h"
#include <cmath>
#include "RegBigFontShxParser.h"
#include "ShxFileMapping.h"
#include "ShxParser.h"
#include <assert.h>
#include <osg/PrimitiveSet>
#include <osg/GLExtensions>
#include <osg/State>
#include <osgUtil/CullVisitor>

#undef DrawText

class ShxTextCallback : public osg::NodeCallback
{
    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
        {
            auto text = static_cast<ShxText*>(node);
            text->build();
        }
        else if(nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
        {
            osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
            auto text = static_cast<ShxText*>(node);
            osg::Matrix matrix;
            text->computeMatrix(matrix, cv->getState());
        }
        // must call any nested node callbacks and continue subgraph traversal.
        NodeCallback::traverse(node, nv);
    }
};

namespace
{
    osg::ref_ptr<ShxTextCallback> s_ShxTextCallback(new ShxTextCallback);
    enum { LEFT, CENTER, RIGHT };
    enum { TOP, VCENTER, BOTTOM };
}

const float ShxText::m_EmHeight = 2048.0f;

void ShxText::setCharacterSize(float height)
{
	assert(height > 0);
    if (_characterHeight != height)
    {
        m_MatrixValid = false;
        _characterHeight = height;
    }
}

void ShxText::setCharacterSize(float height, float widthRatio)
{
    const float minFactor = 0.1f;
    assert(widthRatio >= minFactor);
    if (_widthRatio != widthRatio && widthRatio >= minFactor)
    {
        m_CoordsValid = false;
        //m_EmGlyphLengthValid is unchanged.
        _widthRatio = widthRatio;
        if (m_box && m_box->getCount() == 4)
        {
            int first = m_box->getFirst();
            double len = emLength();
            double margin = _margin / _characterHeight * m_EmHeight;
            (*_coords)[first + 1].x() = (*_coords)[first + 2].x() = len + margin;
        }
    }
    setCharacterSize(height);
}

ShxText::ShxText(void)
    : _color(1, 1, 1)
    , _alignment(LEFT_TOP)
    , m_RegFontFile("txt.shx")
    , m_BigFontFile("chinese.shx")
    , _widthRatio(1.0f)
    , _characterHeight(1)
    , _characterSizeMode(OBJECT_COORDS)
    , _lineSpacing(1.5)
    , _text()
    , _position()
    , _rotation()
    , _autoRotateToScreen(false)
    , _showBox(false)
    , _margin(0.f)
    , _lineCount(1)
    , _coords()
    , _colors()
    , m_primitiveSet(new osg::MultiDrawArrays(GL_LINE_STRIP))
    , m_box(new osg::DrawArrays())
    , m_EmGlyphLengthValid(false)
    , m_CoordsValid(false)
    , m_ColorsValid(false)
    , m_MatrixValid(false)
    , m_startIndex(0)
{
    setUseVertexBufferObjects(true);

    setUpdateCallback(s_ShxTextCallback);
    setCullCallback(s_ShxTextCallback);
}

ShxText::ShxText(const ShxText& st, const osg::CopyOp& copyop)
	: Geometry(st, copyop)
    , _color(st._color)
    , _alignment(st._alignment)
    , m_RegFontFile(st.m_RegFontFile)
    , m_BigFontFile(st.m_BigFontFile)
    , _widthRatio(st._widthRatio)
    , _characterHeight(st._characterHeight)
    , _characterSizeMode(st._characterSizeMode)
    , _lineSpacing(st._lineSpacing)
    , _text(st._text)
    , _position(st._position)
    , _rotation(st._rotation)
    , _autoRotateToScreen(st._autoRotateToScreen)
    , _showBox(st._showBox)
    , _margin(st._margin)
    , _lineCount(st._lineCount)
    , _matrix(st._matrix)
    , _coords(static_cast<osg::Vec2Array*>(copyop(st._coords.get())))
    , _colors(static_cast<osg::Vec3Array*>(copyop(st._colors.get())))
    , m_primitiveSet(static_cast<osg::MultiDrawArrays*>(copyop(st.m_primitiveSet.get())))
    , m_box(static_cast<osg::DrawArrays*>(copyop(st.m_box.get())))
    , m_EmGlyphLengthValid(st.m_EmGlyphLengthValid)
    , m_CoordsValid(st.m_CoordsValid)
    , m_ColorsValid(st.m_ColorsValid)
    , m_MatrixValid(st.m_MatrixValid)
    , m_startIndex(st.m_startIndex)
{
    setVertexArray(_coords);
    setColorArray(_colors);
    addPrimitiveSet(m_primitiveSet);
    if (_showBox)
    {
        addPrimitiveSet(m_box);
    }
    setUseVertexBufferObjects(true);
    
    setUpdateCallback(s_ShxTextCallback);
    setCullCallback(s_ShxTextCallback);
}

ShxText::~ShxText(void)
{
	if(!m_RegFontFile.empty())
		CShxFileMapping::Release(m_RegFontFile);
	if(!m_BigFontFile.empty())
		CShxFileMapping::Release(m_BigFontFile);
}

float ShxText::length() const
{
    const double pp = _characterHeight / m_EmHeight;
    return pp * emLength();
}

void ShxText::setColor(const osg::Vec3& color)
{
    if (_color != color)
    {
        _color = color;
        m_ColorsValid = false;
    }
}

void ShxText::setFontFile(const char* Reg_Uni_ShxFile, const char* Big_ShxFile)
{
	assert(Reg_Uni_ShxFile != nullptr && *Reg_Uni_ShxFile != '\0');
	if (m_RegFontFile == Reg_Uni_ShxFile && m_BigFontFile == Big_ShxFile)
		return;
    m_EmGlyphLengthValid = false;
    m_CoordsValid = false;
	if(!m_RegFontFile.empty())
		CShxFileMapping::Release(m_RegFontFile);
	if(!m_BigFontFile.empty())
		CShxFileMapping::Release(m_BigFontFile);
	m_RegFontFile = Reg_Uni_ShxFile;
	m_BigFontFile = Big_ShxFile;
	CShxFileMapping::AddRef(Reg_Uni_ShxFile);
	CShxFileMapping::AddRef(Big_ShxFile);
}

void ShxText::setText(const std::wstring& text) {
	if (text == _text)
		return;
    m_EmGlyphLengthValid = false;
    m_CoordsValid = false;
    if (LEFT != _alignment / 3)
        m_MatrixValid = false;
    _text = text;

    // _text = _text.trim();
    auto pos = _text.find_first_not_of(L" \t\r\n");
    if (pos != std::wstring::npos)
        _text = _text.substr(pos);
    pos = _text.find_last_not_of(L" \t\r\n");
    if (pos != std::wstring::npos)
        _text = _text.substr(0, pos + 1);

    // record all '\n' positions
    _lineStops.clear();
    pos = 0;
    while ((pos = _text.find_first_of(L'\n', pos)) != std::wstring::npos)
    {
        _lineStops.push_back(int(pos));
        ++pos;
    }
    _lineStops.push_back(int(_text.size()));
    _lineCount = unsigned int(_lineStops.size());
}

void ShxText::calEmGlyph() const
{
	if(m_EmGlyphLengthValid)
		return;

	m_EmGlyphLength = 0;

    if (!_text.empty())
    {
	    CRegBigFontShxParser shxParser;
	    shxParser.Init(m_RegFontFile.c_str(), m_BigFontFile.c_str());
	    shxParser.SetTextHeight(m_EmHeight);
        wchar_t* data = const_cast<wchar_t*>(_text.c_str());
        wchar_t* cur = data;
        _lineEmGlyphLength.resize(size_t(_lineCount));
        for (unsigned int i = 0; i < _lineCount; ++i)
        {
            // Is it safe to change std::wstring temporarily?
            wchar_t c = data[_lineStops[i]];
            if (c != L'\0')
                data[_lineStops[i]] = L'\0';
	        auto textLen = shxParser.GetTextExtent(cur);
            if (textLen > m_EmGlyphLength)
                m_EmGlyphLength = textLen;
            _lineEmGlyphLength[i] = textLen;
            if (c != L'\0')
                data[_lineStops[i]] = c;
            cur = data + _lineStops[i] + 1;
        }
    }

    m_EmGlyphLengthValid = true;
}

osg::Vec3f ShxText::emLeftBottom() const
{
	float dx = 0, dy = 0;
    float incX = emLength();
	switch(_alignment / 3)
	{
	case LEFT:
		dx = 0;
		break;
	case CENTER:
		dx = -0.5*incX;
		break;
	case RIGHT:
		dx = -incX;
		break;
	default:
		assert(false);
		break;
	}
	switch(_alignment % 3)
	{
	case TOP:
		dy = -m_EmHeight * (1 + _lineSpacing * (_lineCount - 1));
		break;
	case VCENTER:
		dy = -0.5 * m_EmHeight * (1 + _lineSpacing * (_lineCount - 1));
		break;
	case BOTTOM:
		dy = 0;
		break;
	default:
		assert(false);
		break;
	}
	return osg::Vec3f(dx, dy, 0);
}

void ShxText::build()
{
    if (!m_CoordsValid)
    {
        m_CoordsValid = true;
        // reset all the properties.
        _coords = new osg::Vec2Array(osg::Array::Binding::BIND_PER_VERTEX);
        if (!_text.empty())
        {
            const_cast<ShxText*>(this)->setVertexArray(_coords);
            m_primitiveSet = new osg::MultiDrawArrays(GL_LINE_STRIP);
            const_cast<ShxText*>(this)->getPrimitiveSetList().clear();
            const_cast<ShxText*>(this)->addPrimitiveSet(m_primitiveSet);
            {
                CRegBigFontShxParser shxParser;
                shxParser.Init(m_RegFontFile.c_str(), m_BigFontFile.c_str());
                shxParser.SetTextHeight(m_EmHeight);
                wchar_t* data = const_cast<wchar_t*>(_text.c_str());
                wchar_t* cur = data;
                for (unsigned int i = 0; i < _lineCount; ++i)
                {
                    // Is it safe to change std::wstring temporarily?
                    wchar_t c = data[_lineStops[i]];
                    if (c != L'\0')
                        data[_lineStops[i]] = L'\0';
                    shxParser.DrawText(static_cast<IGlyphCallback*>(const_cast<ShxText*>(this)), cur, lineXOffset(i), (_lineCount - 1 - i) * _lineSpacing * m_EmHeight);
                    if (c != L'\0')
                        data[_lineStops[i]] = c;
                    cur = data + _lineStops[i] + 1;
                }
            }
            m_box = new osg::DrawArrays();
            int first = int(_coords->size());
            double len = emLength();
            double margin = _margin / _characterHeight * m_EmHeight;
            osg::Vec2f lb2(-margin, -margin);
            _coords->push_back(lb2);
            lb2.x() = len + margin;
            _coords->push_back(lb2);
            lb2.y() = m_EmHeight * (1 + _lineSpacing * (_lineCount - 1)) + margin;
            _coords->push_back(lb2);
            lb2.x() = -margin;
            _coords->push_back(lb2);
            m_box->set(GL_LINE_LOOP, first, 4);
            if (_showBox)
            {
                const_cast<ShxText*>(this)->addPrimitiveSet(m_box);
            }
        }

        _coords->dirty();
    }

    if (!m_ColorsValid)
    {
        m_ColorsValid = true;

        _colors = new osg::Vec3Array(osg::Array::Binding::BIND_OVERALL);
        const_cast<ShxText*>(this)->setColorArray(_colors);
        _colors->push_back(_color);

        _colors->dirty();
    }
}

void ShxText::drawImplementation(osg::RenderInfo& renderInfo) const
{
    if (!m_CoordsValid || !m_ColorsValid || _text.empty())
        return;
    osg::State& state = *renderInfo.getState();

    osg::Matrix modelview;
    if (!m_MatrixValid)
    {
        computeMatrix(modelview, &state);
    }
    else
    {
        modelview = _matrix;
    }
    osg::Matrix previous_modelview = state.getModelViewMatrix();

    // set up the new modelview matrix

    // ** mult previous by the modelview for this context
    modelview.postMult(previous_modelview);

    // ** apply this new modelview matrix
    state.applyModelViewMatrix(modelview);

    // workaround for GL3/GL2
    if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();

    // OSG_NOTICE<<"New state.applyModelViewMatrix() "<<modelview<<std::endl;
    // save the previous modelview matrix
    Geometry::drawImplementation(renderInfo);

    // ** apply this new modelview matrix
    state.applyModelViewMatrix(previous_modelview);

    // workaround for GL3/GL2
    if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();
}

void ShxText::setLineSpacing(float lineSpacing)
{
    if (lineSpacing != _lineSpacing)
    {
        _lineSpacing = lineSpacing;
        if (_lineCount > 1)
            m_MatrixValid = false;
        if (m_box && m_box->getCount() == 4)
        {
            int first = m_box->getFirst();
            osg::Vec2f lb2;
            double margin = _margin / _characterHeight * m_EmHeight;
            (*_coords)[first + 2].y() = (*_coords)[first + 3].y() = m_EmHeight * (1 + _lineSpacing * (_lineCount - 1)) + margin;
        }
    }
}

void ShxText::setPosition(const osg::Vec3 & pos)
{
    if (pos != _position)
    {
        _position = pos;
        m_MatrixValid = false;
    }
}
void ShxText::setAlignment(AlignmentType alignment)
{
    if (alignment != _alignment)
    {
        _alignment = alignment;
        m_MatrixValid = false;
    }
}

void ShxText::setRotation(const osg::Quat & quat)
{
    if (quat != _rotation)
    {
        _rotation = quat;
        m_MatrixValid = false;
    }
}

void ShxText::setAutoRotateToScreen(bool autoRotateToScreen)
{
    if (autoRotateToScreen != _autoRotateToScreen)
    {
        _autoRotateToScreen = autoRotateToScreen;
        m_MatrixValid = false;
    }
}

void ShxText::showBox(bool val)
{
    if (_showBox != val)
    {
        _showBox = val;
        if (val)
        {
            addPrimitiveSet(m_box);
        }
        else
        {
            int num = getPrimitiveSetIndex(m_box);
            removePrimitiveSet(num);
        }
    }
}

void ShxText::setBoxMargin(float m)
{
    if (m != _margin)
    {
        _margin = m;
        if (m_box && m_box->getCount() == 4)
        {
            int first = m_box->getFirst();
            double len = emLength();
            double margin = _margin / _characterHeight * m_EmHeight;
            float y = m_EmHeight * (1 + _lineSpacing * (_lineCount - 1)) + margin;
            (*_coords)[first] = osg::Vec2f(-margin, -margin);
            (*_coords)[first+1] = osg::Vec2f(len + margin, -margin);
            (*_coords)[first+2] = osg::Vec2f(len + margin, y);
            (*_coords)[first+3] = osg::Vec2f(-margin, y);
        }
    }
}

bool ShxText::computeMatrix(osg::Matrix& matrix, osg::State* state) const
{
    osg::Vec3 offset = emLeftBottom();
    const float pp = _characterHeight / m_EmHeight;
    if (state && (_characterSizeMode != OBJECT_COORDS || _autoRotateToScreen))
    {
        // m_MatrixValid always be false in this case
        //m_MatrixValid = true;
        osg::Matrix modelview = state->getModelViewMatrix();
        osg::Matrix projection = state->getProjectionMatrix();

        osg::Matrix temp_matrix(modelview);
        temp_matrix.setTrans(0.0, 0.0, 0.0);

        osg::Matrix rotate_matrix;
        rotate_matrix.invert(temp_matrix);

        matrix.makeTranslate(offset);
        osg::Vec3 scaleVec(pp, pp, pp);
        matrix.postMultScale(scaleVec);
        if (!_rotation.zeroRotation())
            matrix.postMultRotate(_rotation);

        if (_characterSizeMode != OBJECT_COORDS)
        {
            typedef osg::Matrix::value_type value_type;

            value_type width = 1280.0;
            value_type height = 1024.0;

            const osg::Viewport* viewport = state->getCurrentViewport();
            if (viewport)
            {
                width = static_cast<value_type>(viewport->width());
                height = static_cast<value_type>(viewport->height());
            }

            osg::Matrix mvpw = modelview * projection * osg::Matrix::scale(width / 2.0, height / 2.0, 1.0);
            if (_autoRotateToScreen)
                mvpw = rotate_matrix * mvpw;

            osg::Vec3d origin = osg::Vec3d(0.0, 0.0, 0.0) * mvpw;
            osg::Vec3d left = osg::Vec3d(1.0, 0.0, 0.0) * mvpw - origin;
            osg::Vec3d up = osg::Vec3d(0.0, 1.0, 0.0) * mvpw - origin;

            // compute the pixel size vector.
            value_type length_x = left.length();
            value_type scale_x = length_x>0.0 ? 1.0 / length_x : 1.0;

            value_type length_y = up.length();
            value_type scale_y = length_y>0.0 ? 1.0 / length_y : 1.0;

            if (_characterSizeMode == SCREEN_COORDS)
            {
                matrix.postMultScale(osg::Vec3(scale_x, scale_y, scale_x));
            }
            else // OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT
            {
                value_type pixelSizeVert = _characterHeight / scale_y;

                // avoid nasty math by preventing a divide by zero
                if (pixelSizeVert == 0.0)
                    pixelSizeVert = 1.0;

                if (pixelSizeVert > _characterHeight)
                {
                    value_type scale_font = _characterHeight / pixelSizeVert;
                    matrix.postMultScale(osg::Vec3f(scale_font, scale_font, scale_font));
                }
            }
        }

        if (_autoRotateToScreen)
        {
            matrix.postMult(rotate_matrix);
        }

        matrix.postMultTranslate(_position);
    }
    else if (!_rotation.zeroRotation() && !m_MatrixValid)
    {
        if (!m_MatrixValid || (_characterSizeMode != OBJECT_COORDS || _autoRotateToScreen))
        {
            if (_characterSizeMode == OBJECT_COORDS && !_autoRotateToScreen)
                m_MatrixValid = true;
            matrix.makeTranslate(offset);
            osg::Vec3 scaleVec(pp, pp, pp);
            matrix.postMultScale(scaleVec);
            matrix.postMultRotate(_rotation);
            matrix.postMultTranslate(_position);
        }
        else
        {
            matrix = _matrix;
        }
    }
    else
    {
        if (!m_MatrixValid || (_characterSizeMode != OBJECT_COORDS || _autoRotateToScreen))
        {
            if (_characterSizeMode == OBJECT_COORDS && !_autoRotateToScreen)
                m_MatrixValid = true;
            matrix.makeTranslate(offset);
            osg::Vec3 scaleVec(pp, pp, pp);
            matrix.postMultScale(scaleVec);
            matrix.postMultTranslate(_position);
        }
        else
        {
            matrix = _matrix;
        }
    }

    if (_matrix != matrix)
    {
        _matrix = matrix;
        const_cast<ShxText*>(this)->dirtyBound();
    }

    return true;
}

float ShxText::emLength() const
{
	calEmGlyph();
	return m_EmGlyphLength * _widthRatio;
}

double ShxText::lineXOffset(int lineIndex)
{
    calEmGlyph();
    double dx = 0;
    ;
    switch (_alignment / 3)
    {
    case LEFT:
        dx = 0;
        break;
    case CENTER:
        dx = 0.5*(m_EmGlyphLength - _lineEmGlyphLength[lineIndex]);
        break;
    case RIGHT:
        dx = m_EmGlyphLength - _lineEmGlyphLength[lineIndex];
        break;
    default:
        assert(false);
        break;
    }
    return dx;
}

osg::BoundingBox ShxText::computeBoundingBox() const
{
    osg::BoundingBox  bbox;

    if (!m_RegFontFile.empty() && !_text.empty())
    {
        //osg::Matrix modelview;
        //computeMatrix(modelview, nullptr);
        auto len = emLength();
        osg::BoundingBox  tmp;
        tmp.set(0, 0, 0, len, m_EmHeight * (1 + _lineSpacing * (_lineCount - 1)), 0);
        bbox.expandBy(tmp.corner(0)*_matrix);
        bbox.expandBy(tmp.corner(1)*_matrix);
        bbox.expandBy(tmp.corner(2)*_matrix);
        bbox.expandBy(tmp.corner(3)*_matrix);
        bbox.expandBy(tmp.corner(4)*_matrix);
        bbox.expandBy(tmp.corner(5)*_matrix);
        bbox.expandBy(tmp.corner(6)*_matrix);
        bbox.expandBy(tmp.corner(7)*_matrix);
    }

    return bbox;
}

void ShxText::glBegin(int mode)
{
	assert(mode == GL_LINE_STRIP);
	auto _vertices = dynamic_cast<osg::Vec2Array*>(getVertexArray());
	m_startIndex = _vertices->size();
}

void ShxText::glVertex2d(double x, double y)
{
	auto _vertices = dynamic_cast<osg::Vec2Array*>(getVertexArray());
	_vertices->push_back(osg::Vec2(x * _widthRatio, y));
}

void ShxText::glEnd()
{
	auto _vertices = dynamic_cast<osg::Vec2Array*>(getVertexArray());
	m_primitiveSet->add(m_startIndex, _vertices->size() - m_startIndex);
}
