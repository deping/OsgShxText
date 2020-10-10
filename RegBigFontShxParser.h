#pragma once
#include "ShxParser.h"
#include "IGlyphCallback.h"
#include "TextLayout.h"
#include "GdiGlyphCallback.h"

#pragma push_macro("DrawText")
#undef DrawText

class CRegBigFontShxParser
{
public:
	//ShxFileName must be file name without directory. search under \AutoCAD\Fonts relative to program.
	CRegBigFontShxParser(const char* Reg_Uni_ShxFile, const char* Big_ShxFile);
	CRegBigFontShxParser() {}
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
	void DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y);
	void DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y);
	void setLayout(TextLayout layout);
	void setVKerning(double vkerning);

private:
	CShxParser m_RegFontShx;
	CShxParser m_BigFontShx;
	TextLayout m_layout;
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
	WidthGlyphCallback gcb;
	DrawText(&gcb, text, 0, 0);
	return gcb.getWidth();
}

double CRegBigFontShxParser::GetTextExtent(const wchar_t* text)
{
	WidthGlyphCallback gcb;
	DrawText(&gcb, text, 0, 0);
	return gcb.getWidth();
}

#pragma pop_macro("DrawText") 