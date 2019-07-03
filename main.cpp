/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include <osg/Geode>
#include "ShxText.h"
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgUtil/ShaderGen>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/PositionAttitudeTransform>
#include <osgGA/StateSetManipulator>
#include <osg/Math>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>


class MyBBox : public osg::ShapeDrawable
{
public:
    MyBBox(osg::Drawable* drawable = nullptr);
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    MyBBox(const MyBBox& pg, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
        : osg::ShapeDrawable(pg, copyop)
    {
        m_box = new osg::Box(*pg.m_box);
        setShape(m_box);
        m_object = pg.m_object;
    }

    virtual Object* cloneType() const { return new MyBBox(); }
    virtual Object* clone(const osg::CopyOp& copyop) const { return new MyBBox(*this, copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const MyBBox*>(obj) != NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "MyBBox"; }

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        osg::ShapeDrawable::drawImplementation(renderInfo);
    }
    osg::BoundingBox computeBoundingBox() const
    {
        return osg::ShapeDrawable::computeBoundingBox();
    }

    osg::ref_ptr<osg::Box> m_box;
    osg::ref_ptr<osg::Drawable> m_object;
};

class UpdateBBox : public osg::Drawable::UpdateCallback
{
public:
    virtual void update(osg::NodeVisitor*, osg::Drawable* drawable)
    {
        MyBBox* pBBox = dynamic_cast<MyBBox*>(drawable);
        if (pBBox)
        {
            auto bbox = pBBox->m_object->getBoundingBox();
            osg::Vec3 half(bbox.xMax() - bbox.xMin(), bbox.yMax() - bbox.yMin(), bbox.zMax() - bbox.zMin());
            half /= 2.0;
            pBBox->m_box->set(bbox.center(), half);
            pBBox->dirtyBound();
            pBBox->dirtyGLObjects();
            pBBox->build();
        }
    }
};

MyBBox::MyBBox(osg::Drawable* drawable)
{
    auto ss = getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
    m_object = drawable;
    m_box = new osg::Box(osg::Vec3(), 1.0f);
    setShape(m_box.get());
    setUpdateCallback(new UpdateBBox);
}

const int rowColSize = 5;
const int textHeight = 15;
const float lineSpacing = 1.5f;
const float rowHeight = 2 * lineSpacing * textHeight;
const float colWidth = 200;
osg::Group* createTexts()
{
	osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	pGroup->setStateSet(stateset);
	auto strFontFileCn = "C:\\Windows\\Fonts\\simsun.ttc";

	auto font = osgText::readRefFontFile(strFontFileCn);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    for (int i = 0; i < rowColSize; ++i)
    {
        for (int j = 0; j < rowColSize; ++j)
        {
#define TextType ShxText
            osg::ref_ptr<ShxText> pText = new ShxText();
            if (i == 3)
                pText->setCharacterSize(textHeight, 0.75);
            else
                pText->setCharacterSize(textHeight);
            pText->setText(L"Hello, China\n你好, 中国");
            pText->setFontFile("txt.shx", "hztxt.shx");//THFont.shx gbcbig.shx

            //osg::ref_ptr<osg::Geometry> pLine = new osg::Geometry;
            //osg::Vec2Array* pVertexArray = new osg::Vec2Array;
            //pVertexArray->push_back(osg::Vec2(colWidth * i, pText->getLineCount() * rowHeight * j));
            //pVertexArray->push_back(osg::Vec2(colWidth * i + pText->length(), pText->getLineCount() * rowHeight * j));
            //pLine->setVertexArray(pVertexArray);
            //osg::Vec3Array* pColorArray = new osg::Vec3Array(osg::Array::BIND_OVERALL);
            //pColorArray->push_back(osg::Vec3(1, 1, 0));
            //pLine->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 2));
            //geode->addDrawable(pLine);

//#define TextType osgText::Text
            //osg::ref_ptr<osgText::Text> pText = new osgText::Text;
            //pText->setText(L"Hello, China\n你好, 中国");
            //pText->setCharacterSize(10);
            //pText->setFont(font);

            if (i == 0)
            {
                pText->setAlignment(ShxText::AlignmentType::CENTER_CENTER);
                pText->setCharacterSizeMode(TextType::SCREEN_COORDS);
            }
            else if (i == 1)
                pText->setCharacterSizeMode(TextType::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
            else if (i == 2)
            {
                pText->setAlignment(ShxText::AlignmentType::RIGHT_CENTER);
                pText->setCharacterSizeMode(TextType::SCREEN_COORDS);
                pText->setAutoRotateToScreen(true);
            }
            pText->showBox(true);
            pText->setBoxMargin(6);

            if (j == 0)
                pText->setRotation(osg::Quat(osg::PI_4f, osg::Z_AXIS));

            pText->setPosition(osg::Vec3(colWidth * i, pText->getLineCount() * rowHeight * j, 0));

            geode->addDrawable(pText);
            //geode->addDrawable(new MyBBox(pText.get()));
            
       }
    }
    pGroup->addChild(geode);
	return pGroup.release();
}

int main(int argc, char **argv)
{
	// construct the viewer.
	osgViewer::Viewer viewer;
	//viewer.setUpViewInWindow(0, 0, 1920, 540);
    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits;
    traits->x = 50;
    traits->y = 50;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    //traits->samples = 4;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext(traits.get());
    osg::ref_ptr<osg::Camera> camera = viewer.getCamera();
    camera->setGraphicsContext(gc);
    camera->setViewport(
        new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));

	// add model to viewer.
	viewer.setSceneData(createTexts());

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);
	osg::ref_ptr<osgGA::TrackballManipulator> man = new osgGA::TrackballManipulator();
	man->setHomePosition(osg::Vec3d(colWidth * rowColSize / 2.0f, rowHeight * rowColSize / 2.0f, 3 * colWidth * rowColSize), osg::Vec3d(colWidth * rowColSize / 2.0f, rowHeight * rowColSize / 2.0f, 0), osg::Vec3d(0, 1, 0));
	viewer.setCameraManipulator(man.get());
	viewer.realize();

	return viewer.run();
}
