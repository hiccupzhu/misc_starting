#include "StdAfx.h"
#include "Render.h"

#include <string.h>

using namespace Ogre;

CRender::CRender(void)
{
    m_main_hwnd = NULL;
    m_pRoot     = NULL;
    m_pNode     = NULL;
    m_pCamera   = NULL;
    m_pWindow   = NULL;
    m_pSceneMgr = NULL;

}

CRender::~CRender(void)
{
}


int CRender::init(const char* plugin, const char* ogre,const char* log){
    char* _plugin  = NULL;
    char* _ogre    = NULL;
    char* _log     = NULL;
    
    if(plugin){
        _plugin = strdup(plugin);
    }else{
#ifdef _DEBUG
        _plugin = strdup("plugins_d.cfg");
#else
        _plugin = strdup("plugins.cfg");
#endif
    }
    if(ogre){
        _ogre = strdup(ogre);
    }else{
        _ogre = strdup("ogre.cfg");
    }
    if(log){
        _log = strdup(log);
    }else{
        _log = strdup("ogre.log");
    }
    
    m_pRoot = new Ogre::Root(_plugin, _ogre, _log);
    if(m_pRoot ==  NULL)
        return ERR_NO_MEM;
    Ogre::ConfigFile cfg;
#ifdef _DEBUG
    cfg.load("resources_d.cfg");
#else
    cfg.load("resources.cfg");
#endif

    Ogre::ConfigFile::SectionIterator seci=cfg.getSectionIterator();
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
    
    cfg.clear();
#ifdef _DEBUG
    cfg.load("samples.cfg");
#else
    cfg.load("samples.cfg");
#endif  

    Ogre::String sampleDir = cfg.getSetting("SampleFolder");        // Mac OS X just uses Resources/ directory
    Ogre::StringVector sampleList = cfg.getMultiSetting("SamplePlugin");
    
    for (Ogre::StringVector::iterator i = sampleList.begin(); i != sampleList.end(); i++)
    {
        try   // try to load the plugin
        {
            m_pRoot->loadPlugin(sampleDir + *i);
        }
        catch (Ogre::Exception e)   // plugin couldn't be loaded
        {
//             unloadedSamplePlugins.push_back(sampleDir + *i);
//             continue;
            printf("error load \n");
        }

    }



    
    
    bool carrayOn=false;
    carrayOn= m_pRoot->showConfigDialog();
    if(carrayOn)
    {
        m_pRoot->initialise( false );

        Ogre::NameValuePairList miscParams; 
        miscParams["externalWindowHandle"] = Ogre::StringConverter::toString( (int)m_main_hwnd );

        m_pWindow   = m_pRoot->createRenderWindow("123",m_mainrc.Width() , m_mainrc.Height(), false, &miscParams );

        m_pSceneMgr = m_pRoot->createSceneManager(ST_GENERIC,"sceneMgr1");
        m_pCamera   = m_pSceneMgr->createCamera("camera1");
        m_pCamera->setPosition(Ogre::Vector3(0,0,10));
        m_pCamera->lookAt(Ogre::Vector3(0,0,-1));
        m_pCamera->setNearClipDistance(1);

        Ogre::Viewport* vp = m_pWindow->addViewport(m_pCamera);
        vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
        m_pCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
        Ogre::Light* l = m_pSceneMgr->createLight("MainLight");
        l->setPosition(20,80,50);

        //Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, "");
        Ogre::Node* pSceneNode=(Ogre::SceneNode*)m_pSceneMgr->getRootSceneNode()->createChild("scene1");
        
        Ogre::Entity *ent = m_pSceneMgr->createEntity("shape1","skinsShape.mesh");
        //ent->setMaterialName("Examples/EnvMappedRustySteel");
        m_pNode=(Ogre::SceneNode*)pSceneNode->createChild("node1",Ogre::Vector3(0,0,0));
        m_pNode->attachObject(ent);


        //SetWindowsHookEx(WH_MOUSE,MouseProc,NULL,GetCurrentThreadId());

    }
    return 0;
}


int i = 0;

int CRender::RenderFrame(){
#if 0
    if(m_main_hwnd){
        HDC hdc = GetDC(m_main_hwnd);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, i, i);
        i ++;
        if(i >= m_mainrc.Width() || i >= m_mainrc.Height()){
            i = 0;
        }
    }
#endif
    if(m_pRoot)
        m_pRoot->renderOneFrame();
    return 0;
}

int CRender::SetMainHwnd(HWND hwnd){
    m_main_hwnd = hwnd;
    GetClientRect(m_main_hwnd, &m_mainrc);
    return 0;
}

int CRender::GetWidth(RECT& rc) const{
    return rc.right - rc.left;
}

int CRender::GetHeight(RECT& rc) const{
    return rc.bottom - rc.top;
}