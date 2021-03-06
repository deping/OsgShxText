/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
//#include "stdafx.h"
#include <Windows.h>
#include "RegBigFontShxParser.h"
#include <atlconv.h>
#include <gl/GL.h>
#include <assert.h>
#include "IGlyphCallback.h"

#undef DrawText

CRegBigFontShxParser::CRegBigFontShxParser(const char* Reg_Uni_ShxFile, const char* Big_ShxFile)
	:m_RegFontShx(Reg_Uni_ShxFile)
	, m_BigFontShx(Big_ShxFile)
{
	if (m_RegFontShx.m_Type == SHAPEFILE)
		assert(m_BigFontShx.m_Type == UNKNOWN);
	else
	{
		assert(m_RegFontShx.m_Type == UNKNOWN || m_RegFontShx.m_Type == REGFONT || m_RegFontShx.m_Type == UNIFONT);
		assert(m_BigFontShx.m_Type == UNKNOWN || m_BigFontShx.m_Type == BIGFONT);
	}
}

void CRegBigFontShxParser::Init(const char* Reg_Uni_ShxFile, const char* Big_ShxFile)
{
	m_RegFontShx.Init(Reg_Uni_ShxFile);
	m_BigFontShx.Init(Big_ShxFile);

	if (m_RegFontShx.m_Type == SHAPEFILE)
		assert(m_BigFontShx.m_Type == UNKNOWN);
	else
	{
		assert(m_RegFontShx.m_Type == UNKNOWN || m_RegFontShx.m_Type == REGFONT || m_RegFontShx.m_Type == UNIFONT);
		assert(m_BigFontShx.m_Type == UNKNOWN || m_BigFontShx.m_Type == BIGFONT);
	}
}

void CRegBigFontShxParser::Cleanup()
{
	m_RegFontShx.Cleanup();
	m_BigFontShx.Cleanup();
}

CRegBigFontShxParser::~CRegBigFontShxParser(void)
{
}

void CRegBigFontShxParser::DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y)
{
	//sync pen position in 2 files
	m_BigFontShx.m_PenX = m_RegFontShx.m_PenX = x;
	double vh = GetTextHeight() * m_verticalKerning;
	if (m_layout == TextLayout::VERTICAL)
	{
		y -= vh;
	}
	m_BigFontShx.m_PenY = m_RegFontShx.m_PenY = y;
	const double y0 = y;

	//initialize scale
	m_BigFontShx.m_Scale = m_BigFontShx.m_TextHeight / m_BigFontShx.m_FontHeight;
	m_RegFontShx.m_Scale = m_RegFontShx.m_TextHeight / m_RegFontShx.m_FontHeight;

	if (pGlyphCallback) {
		pGlyphCallback->glBegin(GL_LINE_STRIP);
		pGlyphCallback->glVertex2d(x, y);
	}

	bool draw = false;
	while (*text != 0)
	{
		if (m_BigFontShx.m_Type != UNKNOWN && m_BigFontShx.IsEscapeChar(*(unsigned char*)text))
		{
			//here we need revert 2 bytes, then pass to ParseGlyph
			char first = *text;
			char second = *(text + 1);
			unsigned short character = MAKEWORD(second, first);
			m_BigFontShx.ParseGlyph(pGlyphCallback, /**(unsigned short*)text*/character);
			text += 2;
			if (m_layout == TextLayout::VERTICAL)
			{
				m_BigFontShx.m_PenX = x;
				y -= vh;
				m_BigFontShx.m_PenY = y;
			}
			//sync pen position in 2 files
			m_RegFontShx.m_PenX = m_BigFontShx.m_PenX;
			m_RegFontShx.m_PenY = m_BigFontShx.m_PenY;
			//sync scale in 2 files
			m_RegFontShx.m_Scale = (m_RegFontShx.m_TextHeight / m_RegFontShx.m_FontHeight)*
				(m_BigFontShx.m_Scale / (m_BigFontShx.m_TextHeight / m_BigFontShx.m_FontHeight));
			draw = m_BigFontShx.drawMode();
		}
		else
		{
			m_RegFontShx.ParseGlyph(pGlyphCallback, *(unsigned char*)text);
			++text;
			if (m_layout == TextLayout::VERTICAL)
			{
				m_RegFontShx.m_PenX = x;
				y -= vh;
				m_RegFontShx.m_PenY = y;
			}
			//sync pen position in 2 files
			m_BigFontShx.m_PenX = m_RegFontShx.m_PenX;
			m_BigFontShx.m_PenY = m_RegFontShx.m_PenY;
			//sync scale in 2 files
			m_BigFontShx.m_Scale = (m_BigFontShx.m_TextHeight / m_BigFontShx.m_FontHeight)*
				(m_RegFontShx.m_Scale / (m_RegFontShx.m_TextHeight / m_RegFontShx.m_FontHeight));
			draw = m_RegFontShx.drawMode();
		}
		if (m_layout == TextLayout::VERTICAL && !draw && pGlyphCallback) {
			// some English letters such as 'i', 'n', they use the end point of last letter as the start point.
			// so when layout is vertical, their shapes will be wrong.
			// this if statement fixes the problem.
			// see CShxParser::DrawLine
			pGlyphCallback->glEnd();
			pGlyphCallback->glBegin(GL_LINE_STRIP);
			pGlyphCallback->glVertex2d(m_RegFontShx.m_PenX, m_RegFontShx.m_PenY);
		}
	}
	if (pGlyphCallback)
	{
		pGlyphCallback->glEnd();
	}

	//switch (m_layout)
	//{
	//case TextLayout::LEFT_TO_RIGHT:
	//	return m_RegFontShx.m_PenX - x;
	//case TextLayout::VERTICAL:
	//	return y0 - m_RegFontShx.m_PenY;
	//}
	//return 0;
}

void CRegBigFontShxParser::DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y)
{
	USES_CONVERSION;
	char* atext = W2A(text);
	DrawText(pGlyphCallback, atext, x, y);
}

void CRegBigFontShxParser::setLayout(TextLayout layout)
{
	// RIGHT_TO_LEFT not implemented
	if (layout == TextLayout::RIGHT_TO_LEFT)
		return;
	m_layout = layout;
}

void CRegBigFontShxParser::setVKerning(double vkerning)
{
	m_verticalKerning = vkerning;
}
