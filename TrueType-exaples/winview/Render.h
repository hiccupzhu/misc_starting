#pragma once

#include <atltypes.h>
#include <ogre/OgreRoot.h>
#include <ogre/ExampleFrameListener.h>
#include <ogre/OgreConfigFile.h>
#include <Ogre/OgreSceneManager.h>
#include <ogre/OgreCommon.h>
#include <ogre/OgreRenderSystem.h>
#include "erron.h"
#include "MRect.h"

class CRender
{
public:
    CRender(void);
    ~CRender(void);
    
public:
    int     init(const char* plugin = NULL, const char* setting = NULL, const char* log = NULL);
    int     RenderFrame(); 
    int     SetMainHwnd(HWND hwnd);
    
    
    int     GetWidth(RECT& rc) const;
    int     GetHeight(RECT& rc) const;
    
private:
    HWND                m_main_hwnd;
    MRect               m_mainrc;
    Ogre::Root *        m_pRoot;
    Ogre::SceneNode*    m_pNode;
    Ogre::Camera*       m_pCamera;
    Ogre::RenderWindow* m_pWindow;
    Ogre::SceneManager* m_pSceneMgr;
    
};
