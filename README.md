
# OsgShxText
OsgShxText provide OpenSceneGraph(OSG) text node which use AutoCAD shx font.

![ShxText example picture](readme.png)

**Note to build this example with CMake**:
1. Need OSG version >= 3.6
2. Put these files in OpenSceneGraph-3.6.2\examples\osgshxtext
3. Modify OpenSceneGraph-3.6.2\examples\CMakeLists.txt:
   Add the below line:
    ADD_SUBDIRECTORY(osgshxtext)
4. Put shx font file in ACAD\Fonts relative to osg executable path, such as OpenSceneGraph-3.6.2\build\bin\ACAD\Fonts

It's usage is similar to osgText::Text:
```cpp
auto strFontFileCn = "C:\\Windows\\Fonts\\simsun.ttc";
auto font = osgText::readRefFontFile(strFontFileCn);
osg::ref_ptr<osg::Geode> geode = new osg::Geode();
for (int i = 0; i < rowColSize; ++i)
{
    for (int j = 0; j < rowColSize; ++j)
    {
#define TextType ShxText
        osg::ref_ptr<ShxText> pText = new ShxText();
        pText->setCharacterSize(15);
        pText->SetText(L"Hello, 中国");
        pText->SetFontFile("hztxt_e.shx", "hztxt.shx");//THFont.shx gbcbig.shx

        osg::ref_ptr<osg::Geometry> pLine = new osg::Geometry;
        osg::Vec2Array* pVertexArray = new osg::Vec2Array;
        pVertexArray->push_back(osg::Vec2(100 * i, 20 * j));
        pVertexArray->push_back(osg::Vec2(100 * i + pText->length(), 20 * j + 0));
        pLine->setVertexArray(pVertexArray);
        osg::Vec3Array* pColorArray = new osg::Vec3Array(osg::Array::BIND_OVERALL);
        pColorArray->push_back(osg::Vec3(1, 1, 0));
        pLine->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 2));
        geode->addDrawable(pLine);

//#define TextType osgText::Text
        //osg::ref_ptr<osgText::Text> pText = new osgText::Text;
        //pText->setText(L"Hello, 中国");
        //pText->setCharacterSize(10);
        //pText->setFont(font);

        if (i == 0)
            pText->setCharacterSizeMode(TextType::SCREEN_COORDS);
        else if (i == 1)
            pText->setCharacterSizeMode(TextType::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

        if (j == 0)
            pText->setRotation(osg::Quat(osg::PI_4f, osg::Z_AXIS));

        pText->setPosition(osg::Vec3(100 * i, 20 * j, 0));

        geode->addDrawable(pText);
    }
}
```
It contains a shx font parser. If you provide a IGlyphCallback implementation, you can draw shx font in any graphics system such as GDI or DirectX. See GdiGlyphCallback.cpp for example.


