#include "mogre.h"
#include <OgrePlugin.h>
#include <SamplePlugin.h>

#pragma comment(lib,"ogremain_d")
#pragma comment(lib,"OIS_d")
#pragma comment(lib,"freetype_d")

#define CAMERA         "zsqCamera"
#define SCENE_MANAGER  "zsqSceneManager"
#define LIGHT          "zsqLight"
#define RENER_WINDOW   "zsqMainWindow"

using namespace Ogre;
using namespace OgreBites;

#define _ORENGL

mogre::mogre(void)
{
    m_proot          = NULL;
    m_pscene_manager = NULL;
    m_pcamera        = NULL;
    m_plight         = NULL;
    m_pwindow        = NULL;

    m_mouse          = NULL;
    m_keyboard       = NULL;
    m_input_manager  = NULL;
    m_mouse_auto     = 1;
    m_lpressed       = 0;
    m_pnode          = 0;
}

mogre::~mogre(void)
{
    SAFE_DELETE(m_ptrayMgr);
    SAFE_DELETE(m_proot);
}

int mogre::create_input_device(){
    OIS::ParamList pl;
    size_t winHandle = 0;
    std::ostringstream winHandleStr;

    m_pwindow->getCustomAttribute("WINDOW", &winHandle);
    winHandleStr << winHandle;

    pl.insert(std::make_pair("WINDOW", winHandleStr.str()));

    m_input_manager = OIS::InputManager::createInputSystem(pl);

    m_mouse = static_cast<OIS::Mouse*>(m_input_manager->createInputObject(OIS::OISMouse, true));
    m_keyboard = static_cast<OIS::Keyboard*>(m_input_manager->createInputObject(OIS::OISKeyboard, true));

    m_mouse->setEventCallback(this);
    m_keyboard->setEventCallback(this);

    if(m_pwindow){
        const OIS::MouseState& ms = m_mouse->getMouseState();
        ms.height = m_pwindow->getHeight();
        ms.width  = m_pwindow->getWidth();
    }

    m_proot->addFrameListener(this);
    Ogre::WindowEventUtilities::addWindowEventListener(m_pwindow, this);

    return 0;
}

int mogre::create_overlay(){
    int res = 0;
    m_ptrayMgr = new OgreBites::SdkTrayManager("zsqTrayMgr", m_pwindow, m_mouse, this);
    //m_ptrayMgr->showFrameStats(TL_BOTTOMLEFT);
    return res;
}

int mogre::init(){
    int res = 0;
    res = create_root();
    locateResources();
    CHECK(res);
    res = create_scene_manager();
    CHECK(res);
    res = create_light();
    CHECK(res);
    res = create_camera();
    CHECK(res);
    res = create_input_device();
    CHECK(res);

    

    res = create_overlay();
    CHECK(res);



    return res;
}

int mogre::init_sample(){
    Root::PluginInstanceList ip = m_proot->getInstalledPlugins();
    for (Ogre::Root::PluginInstanceList::iterator k = ip.begin(); k != ip.end(); k++)
    {
        if ((*k)->getName() == "Sky Box Sample")
        {
            Ogre::Plugin* p = (*k);
            SamplePlugin* sp = dynamic_cast<SamplePlugin*>(p);
            SampleSet newSamples = sp->getSamples();
            int count = newSamples.size();
            for (SampleSet::iterator j = newSamples.begin(); j != newSamples.end(); j++)
            {
                (*j)->_setup(m_pwindow, m_keyboard, m_mouse, m_FSLayer);
            }
            
            break;
        }
    }
    return 0;
}

int mogre::create_root(){
    int res = 0;
    m_FSLayer = OGRE_NEW_T(FileSystemLayerImpl, Ogre::MEMCATEGORY_GENERAL)(OGRE_VERSION_NAME);
    m_proot = new Ogre::Root();
    
    res = load_plugins();
    CHECK(res);

    RenderSystemList render_list = m_proot->getAvailableRenderers();
    RenderSystemList::iterator it_render;

    for(it_render = render_list.begin(); it_render != render_list.end(); it_render++){
#ifdef _ORENGL
        if((*it_render)->getName() == "OpenGL Rendering Subsystem"){
#else
        if((*it_render)->getName() == "Direct3D9 Rendering Subsystem"){
#endif
            m_proot->setRenderSystem(*it_render);
            break;
        }
    }

    if(m_proot->getRenderSystem() == NULL){
        printf("set render system failed\n");
        return -1;
    }

    m_proot->initialise(false);
    m_pwindow = m_proot->createRenderWindow(RENER_WINDOW, 1000, 750, false, NULL);

    

    return res;
}

int mogre::create_scene_manager(){
    int res = 0;
    if(m_proot){
        m_pscene_manager = m_proot->createSceneManager(ST_GENERIC,SCENE_MANAGER);

        m_pscene_manager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));//设置环境光
    }
    return res;
}

int mogre::create_light(){
    int res = 0;
    if(m_pscene_manager){
//        m_plight = m_pscene_manager->createLight(LIGHT);

//         m_plight->setType(Light::LT_POINT);//设置光源类型为：点光源
//         m_plight->setPosition(Ogre::Vector3(100,100,100));//光源位置
//         m_plight->setDiffuseColour(1.0,0.0,0.0);//散射光
    }

    return res;
}

int mogre::create_camera(){
    int res = 0;
    if(m_pscene_manager){
        m_pcamera = m_pscene_manager->createCamera(CAMERA);

        //m_pcamera->setFOVy(Radian(30.0f));
        m_pcamera->setPosition(0.0f, 0.0f, 300.0f);
        m_pcamera->setNearClipDistance(1.0f);
        m_pcamera->setFarClipDistance(1000.0f);

        m_pcamera->lookAt(0.0f, 0.0f, -1.0f);

        m_pcamera->setAspectRatio(640/480.0);

        Viewport *vp=m_pwindow->addViewport(m_pcamera);//添加摄像机
        vp->setBackgroundColour(ColourValue(0.0,0.0,0.0));//视口背景色
        
    }

    return 0;
}

void mogre::locateResources(){
    // load resource paths from config file
//     Ogre::ConfigFile cf;
//     cf.load(m_FSLayer->getConfigFilePath("resources.cfg"));
// 
//     Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
//     Ogre::String sec, type, arch;
// 
//     // go through all specified resource groups
//     while (seci.hasMoreElements())
//     {
//         sec = seci.peekNextKey();
//         Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
//         Ogre::ConfigFile::SettingsMultiMap::iterator i;
// 
//         // go through all resource paths
//         for (i = settings->begin(); i != settings->end(); i++)
//         {
//             type = i->first;
//             arch = i->second;
// 
// #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
//             // OS X does not set the working directory relative to the app,
//             // In order to make things portable on OS X we need to provide
//             // the loading with it's own bundle path location
//             if (!Ogre::StringUtil::startsWith(arch, "/", false)) // only adjust relative dirs
//                 arch = Ogre::String(Ogre::macBundlePath() + "/" + arch);
// #endif
//             Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
//         }
//     }

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem");
    char path[1024] = {0};
    if(GetModuleFileNameA(GetModuleHandleA(NULL), path, 1024) > 0){

        char* p = NULL;
        p = strrchr(path, '\\');
        if(p){
            *p = 0;
        }
        while ( p = strchr(path, '\\')){
            *p = '/';
        }
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(path, "FileSystem");
    }
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

int mogre::load_entity(){
    if(m_pscene_manager){
        //Entity* e = m_pscene_manager->createEntity("zsqEntity", "ogrehead.mesh");
        Entity* e = m_pscene_manager->createEntity("zsqEntity", "test4.mesh");
        int num = e->getNumSubEntities();
        Ogre::MeshPtr pmesh = e->getMesh();
        Ogre::String meshName = pmesh->getName();
        int count_msh = pmesh->getNumSubMeshes();

        
        
        for(int i = 0; i < num; i++){
            meshName.clear();
            Ogre::SubEntity* sube = e->getSubEntity(i);
            

            pmesh->nameSubMesh(meshName, i);

        }
        m_pnode = m_pscene_manager->getRootSceneNode()->createChildSceneNode("node1");
       
//         double scale = 0.1;
//         m_pnode->setScale(scale, scale, scale);
//         m_pnode->setPosition(0, 50, 0);
        m_pnode->attachObject(e);

        //m_pscene_manager->setSkyBox(true, "Examples/SpaceSkyBox", 500);
        m_pscene_manager->setSkyDome(true, "Examples/CloudySky", 10, 8 , 500);

        MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, -30), 1000, 1000, 10, 10, true, 1, 8, 8, Ogre::Vector3::UNIT_Z);

        // create a floor entity, give it a material, and place it at the origin
        Entity* floor = m_pscene_manager->createEntity("Floor", "floor");
        floor->setMaterialName("Examples/BumpyMetal");
        m_pscene_manager->getRootSceneNode()->attachObject(floor);
    }
    return 0;
}

int mogre::load_plugins(){
    if(m_proot){
#ifdef _ORENGL
        m_proot->loadPlugin("RenderSystem_GL_d");
#else
        m_proot->loadPlugin("RenderSystem_Direct3D9_d");
#endif
//         m_proot->loadPlugin("Plugin_ParticleFX_d");
//         m_proot->loadPlugin("Plugin_BSPSceneManager_d");
//         m_proot->loadPlugin("Plugin_CgProgramManager_d");
//         m_proot->loadPlugin("Plugin_PCZSceneManager_d");
//         m_proot->loadPlugin("Plugin_OctreeZone_d");
//         m_proot->loadPlugin("Plugin_OctreeSceneManager_d");
//        m_proot->loadPlugin("Sample_SkyBox_d");
        
    }
    return 0;
}

void mogre::render_one_frame(){
    if(m_proot && m_pscene_manager){
        m_proot->renderOneFrame();
    }
}

void mogre::render_start(){
    if(m_proot){
        m_proot->startRendering();
    }
}

bool mogre::mouseMoved( const MouseEvent &evt ){
    int res = 0;
    if(m_pcamera){
        if( m_mouse_auto){
            m_pcamera->pitch(Ogre::Degree(-evt.state.Y.rel * 0.05f));
            m_pcamera->yaw(Ogre::Degree(-evt.state.X.rel * 0.05f));
        }else{
            if(m_lpressed == 0){
                m_ptrayMgr->refreshCursor();
            }else{
                Ogre::Real dist = (m_pcamera->getPosition() - m_pnode->_getDerivedPosition()).length();
                m_pcamera->moveRelative(Ogre::Vector3(0, 0, -evt.state.Z.rel * 0.0008f * dist));
                m_pcamera->setPosition(m_pnode->_getDerivedPosition());

                m_pcamera->yaw(Ogre::Degree(-evt.state.X.rel * 0.25f));
                m_pcamera->pitch(Ogre::Degree(-evt.state.Y.rel * 0.25f));

                m_pcamera->moveRelative(Ogre::Vector3(0, 0, dist));
            }
        }
    }
    return true;
}

bool mogre::mousePressed( const MouseEvent &arg, MouseButtonID id ){
    int res = 0;
    if(id == MB_Left){
        m_lpressed = 1;
    }
    return true;
}

bool mogre::mouseReleased( const MouseEvent &arg, MouseButtonID id ){
    int res = 0;
    if(id == MB_Left){
        m_lpressed = 0;
    }
    return true;
}

bool mogre::keyPressed( const KeyEvent &arg ){
    int res = 0;
    if(m_pcamera ){
        m_campos = m_pcamera->getPosition();
        switch(arg.text){
            case 'a':
            case 'A':
                m_campos.x -= 1;
                break;
            case 'd':
            case 'D':
                m_campos.x += 1;
                break;
            case 'w':
            case 'W':
                m_campos.z -= 1;
                break;
            case 's':
            case 'S':
                m_campos.z += 1;
                break;
            default:
                break;
        }
        m_pcamera->setPosition(m_campos);
    }
    if(arg.text == KC_RBRACKET){
        m_mouse_auto = !m_mouse_auto;
    }
    return true;
}

bool mogre::keyReleased( const KeyEvent &arg ){
    int res = 0;
    return true;
}	

void mogre::captureInputDevices(){

    if(m_keyboard)
        m_keyboard->capture();
    if(m_mouse)
        m_mouse->capture();
}

void mogre::windowResized(RenderWindow* rw){
    if(m_mouse){
        const OIS::MouseState& ms = m_mouse->getMouseState();
        ms.width = rw->getWidth();
        ms.height = rw->getHeight();
    }
}

bool mogre::frameStarted(const Ogre::FrameEvent& evt)
{
    captureInputDevices();      // capture input
    
    if(m_pnode){
        m_pnode->yaw(Ogre::Radian(-0.002));
    }

    if(m_pcamera){
        m_campos = m_pcamera->getPosition();
        if(m_keyboard->isKeyDown(KC_A)){
            m_campos.x -= 0.1;
        }else if(m_keyboard->isKeyDown(KC_D)){
            m_campos.x += 0.1;
        }else if(m_keyboard->isKeyDown(KC_W)){
            m_campos.z -= 0.1;
        }else if(m_keyboard->isKeyDown(KC_S)){
            m_campos.z += 0.1;
        }
        m_pcamera->setPosition(m_campos);

    }
    return FrameListener::frameStarted(evt);
}