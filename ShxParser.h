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
#include <stack>
#include "IGlyphCallback.h"
#include "TextLayout.h"

enum SHX_TYPE {REGFONT, UNIFONT, BIGFONT, SHAPEFILE, UNKNOWN};
struct EscapeRange
{
	unsigned char start;
	unsigned char end;
};

#pragma push_macro("DrawText")
#undef DrawText

class CShxFileMapping;
class CShxParser
{
public:
	//ShxFileName must be valid ACAD font file(.shx)
	//ShxFileName must be file name without directory
	CShxParser(const char* ShxFileName);
	CShxParser();
	void Init(const char* ShxFileName);
	void Cleanup();
	~CShxParser(void);
	inline void SetTextHeight(double height);
	inline double GetTextHeight();
	//text length at current text height and font
	inline double GetTextExtent(const char* text);
	inline double GetTextExtent(const wchar_t* text);
	//draw text from left bottom (x,y)
	double DrawText(IGlyphCallback* pGlyphCallback, const char* text, double x, double y);
	double DrawText(IGlyphCallback* pGlyphCallback, const wchar_t* text, double x, double y);
	SHX_TYPE GetType(){return m_Type;}
	void setLayout(Layout layout);
	Layout getLayout() const { return m_layout; }
	void setVKerning(double vkerning);
	double getVKerning() const { return _verticalKerning; }
	bool drawMode() const { return m_bDrawMode; }

private:
	//Get character width. if pGlyphCallback isn't null, draw it
	void ParseGlyph(IGlyphCallback* pGlyphCallback, int character);
	void ParseDefBytes(IGlyphCallback* pGlyphCallback, const unsigned char* pDefBytes, int defbytes);
	void ParseOneCode(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	bool Case_Code_8(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	bool Case_Code_C(IGlyphCallback* pGlyphCallback, const unsigned char*& pDefBytes, int& defbytes);
	void ParseLenDirByte(IGlyphCallback* pGlyphCallback, unsigned char thebyte);
	bool IsEscapeChar(unsigned char character);
	void DrawLine(IGlyphCallback* pGlyphCallback);
	double verticalHeight() const { return m_TextHeight * _verticalKerning; }

	double m_TextHeight;//text upper height
	unsigned int m_DescendHeight;//text lower height recorded in font file
	unsigned int m_FontHeight;//text height recorded in font file
	double m_Scale;//scale on width
	SHX_TYPE m_Type;//Shx font file type
	std::vector<EscapeRange> m_Ranges;//if it is  BIGFONT file, m_Ranges records escape code range
	bool m_bDrawMode;//control draw mode in parsing shape defintion bytes
	double m_PenX, m_PenY;//current pen position which is updated before adjusting scale or finishing shape definition, see method ParseOneCode
	std::stack<double> m_PenPosStack;//pen position stack
	const unsigned char* m_pStart;//memory address which maps file beginning
	const unsigned char* m_pEnd;//memory address which maps file end
	union
	{
		const unsigned char* m_pShapeDefs;//if it is UNIFONT shx font file, the first shape definition
		const unsigned short* m_pIndice;//if it is REGFONT or BIGFONT shx font file, the first entry of index table of shape definitions
	};
	union
	{
		int m_IndexCount;//index entry count
		int m_GlyphCount;//character count
	};
	std::string m_ShxFileName;
	Layout m_layout;
	double _verticalKerning;

public:
	//The next 2 functions traverse all characters in font file
	void ResetNextGlyph();//go to the first character
	//parse current character, and go to next character
	bool ShowNextGlyph(IGlyphCallback* pGlyphCallback, double x, double y);
private:
	union
	{
		const unsigned char* m_pCurrentShapeDef;//if it is UNIFONT shx font file, it points to next shape definition
		const unsigned short* m_pCurrentIndice;//if it is REGFONT or BIGFONT shx font file, it points to next entry of the index table of shape definitions
	};
	int m_NextCharDefOffsetFromFirstShapeDef;//only used by REGFONT font file
	int m_CurrentCount;
	friend class CRegBigFontShxParser;
};

void CShxParser::SetTextHeight(double height)
{
	//assert(height > 0.0);
	m_TextHeight = height>0.0f?height:-height;
}

double CShxParser::GetTextHeight()
{
	return m_TextHeight;
}

double CShxParser::GetTextExtent(const char* text)
{
	return DrawText(nullptr, text, 0, 0);
}

double CShxParser::GetTextExtent(const wchar_t* text)
{
	return DrawText(nullptr, text, 0, 0);
}
#pragma pop_macro("DrawText") 