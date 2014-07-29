#ifndef __MOGRE_H__
#define __MOGRE_H__

#include <Ogre.h>
//#include <OgreFrameListener.h>
#include <OIS.h>
#include "FileSystemLayerImpl.h"

#include "def.h"
#include "SdkTrays.h"

using namespace OIS;
using namespace Ogre;

class mogre : public Ogre::FrameListener,
    public OIS::MouseListener, 
    public OIS::KeyListener ,
    public Ogre::WindowEventListener,
    public OgreBites::SdkTrayListener
{
public:
    mogre(void);
    ~mogre(void);

private:
    int create_root();
    int create_scene_manager();
    int create_light();
    int create_camera();
    int create_input_device();
    int create_overlay();
    

    int load_plugins();

public:
    int init();
    int load_entity();
    int init_sample();
    void render_one_frame();
    void render_start();


public:
    virtual bool mouseMoved( const MouseEvent &arg );
    virtual bool mousePressed( const MouseEvent &arg, MouseButtonID id );
    virtual bool mouseReleased( const MouseEvent &arg, MouseButtonID id );

    virtual bool keyPressed( const KeyEvent &arg );
    virtual bool keyReleased( const KeyEvent &arg );
    virtual void captureInputDevices();
    virtual bool frameStarted(const Ogre::FrameEvent& evt);
    virtual void windowResized(RenderWindow* rw);
    virtual void locateResources();

   

private:
    OgreBites::FileSystemLayer*    m_FSLayer;
    Ogre::Root*         m_proot;
    Ogre::SceneManager* m_pscene_manager;
    Ogre::Camera*       m_pcamera;
    Ogre::Light*        m_plight;
    Ogre::RenderWindow* m_pwindow;

    OIS::InputManager*  m_input_manager;
    OIS::Mouse*         m_mouse;
    OIS::Keyboard*      m_keyboard;

    Ogre::Vector3       m_campos;
    SceneNode*          m_pnode;

    OgreBites::SdkTrayManager* m_ptrayMgr;

    int                 m_mouse_auto;
    int                 m_lpressed;
};


#endif