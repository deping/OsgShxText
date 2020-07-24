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
#include "ShxParser.h"
#include "IGlyphCallback.h"
#include "TextLayout.h"

#pragma push_macro("DrawText")
#undef DrawText

class CRegBigFontShxParser
{
public:
	//ShxFileName must be file name without directory. search under \AutoCAD\Fonts relative to program.
	CRegBigFontShxParser(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);
	CRegBigFontShxParser(){}
	void Init(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);
	void Cleanup();
	~CRegBigFontShxParser(void);
	inline void SetTextHeight(double height);
	inline double GetTextHeight();
	inline double GetDescendHeight();
	//text length at current text height and font
	inline double GetTextExtent(const char* text);
	inline double GetTextExtent(const wchar_t* text);
	//draw text from left bottom (x,y)
	double DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y);
	double DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y);
	void setLayout(Layout layout);
	void setVKerning(double vkerning);

private:
	CShxParser m_RegFontShx;
	CShxParser m_BigFontShx;
	Layout m_layout;
	double m_verticalKerning;
};

void CRegBigFontShxParser::SetTextHeight(double height)
{
	//assert(height > 0.0);
	m_RegFontShx.SetTextHeight(height);
	m_BigFontShx.SetTextHeight(height);
}

double CRegBigFontShxParser::GetTextHeight()
{
	return m_RegFontShx.GetTextHeight();
}

double CRegBigFontShxParser::GetDescendHeight()
{
	return m_RegFontShx.m_DescendHeight * m_RegFontShx.m_TextHeight / m_RegFontShx.m_FontHeight; 
}

double CRegBigFontShxParser::GetTextExtent(const char* text)
{
	return DrawText(false, text, 0, 0);
}

double CRegBigFontShxParser::GetTextExtent(const wchar_t* text)
{
	return DrawText(false, text, 0, 0);
}
#pragma pop_macro("DrawText") 