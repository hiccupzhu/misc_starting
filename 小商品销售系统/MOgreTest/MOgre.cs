using System;
using System.Collections.Generic;
using System.Text;
using Mogre;
using System.Drawing;

namespace MOgreTest
{
    public class OgreWindow
    {
        private int cubeCount = 5;
        private MaterialPtr material = null;
        public Root root;
        public SceneManager sceneMgr;

        protected Camera camera;
        protected Viewport viewport;
        protected RenderWindow window;
        protected IntPtr hWnd;

        public OgreWindow(IntPtr hWnd)
        {
            this.hWnd = hWnd;
        }

        public void InitMogre()
        {            

            //----------------------------------------------------- 
            // 1 enter ogre 
            //----------------------------------------------------- 
            root = new Root();

            //----------------------------------------------------- 
            // 2 configure resource paths
            //----------------------------------------------------- 
            ConfigFile cf = new ConfigFile();
            cf.Load("resources.cfg", "\t:=", true);

            // Go through all sections & settings in the file
            ConfigFile.SectionIterator seci = cf.GetSectionIterator();

            String secName, typeName, archName;

            // Normally we would use the foreach syntax, which enumerates the values, but in this case we need CurrentKey too;
            while (seci.MoveNext())
            {
                secName = seci.CurrentKey;
                ConfigFile.SettingsMultiMap settings = seci.Current;
                foreach (KeyValuePair<string, string> pair in settings)
                {
                    typeName = pair.Key;
                    archName = pair.Value;
                    ResourceGroupManager.Singleton.AddResourceLocation(archName, typeName, secName);
                }
            }

            //----------------------------------------------------- 
            // 3 Configures the application and creates the window
            //----------------------------------------------------- 
            bool foundit = false;
            foreach (RenderSystem rs in root.GetAvailableRenderers())
            {
                root.RenderSystem = rs;
                String rname = root.RenderSystem.Name;
                if (rname == "Direct3D9 Rendering Subsystem")
                {
                    foundit = true;
                    break;
                }
            }

            if (!foundit)
                return; //we didn't find it... Raise exception?

            //we found it, we might as well use it!
            root.RenderSystem.SetConfigOption("Full Screen", "No");
            root.RenderSystem.SetConfigOption("Video Mode", "640 x 480 @ 32-bit colour");

            root.Initialise(false);
            NameValuePairList misc = new NameValuePairList();
            misc["externalWindowHandle"] = hWnd.ToString();
            window = root.CreateRenderWindow("Simple Mogre Form Window", 0, 0, false, misc);
            ResourceGroupManager.Singleton.InitialiseAllResourceGroups();

            //----------------------------------------------------- 
            // 4 Create the SceneManager
            // 
            //		ST_GENERIC = octree
            //		ST_EXTERIOR_CLOSE = simple terrain
            //		ST_EXTERIOR_FAR = nature terrain (depreciated)
            //		ST_EXTERIOR_REAL_FAR = paging landscape
            //		ST_INTERIOR = Quake3 BSP
            //----------------------------------------------------- 
            sceneMgr = root.CreateSceneManager(SceneType.ST_GENERIC, "SceneMgr");
            sceneMgr.AmbientLight = new ColourValue(1f, 1f, 1f);

            //----------------------------------------------------- 
            // 5 Create the camera 
            //----------------------------------------------------- 
            camera = sceneMgr.CreateCamera("SimpleCamera");
            camera.Position = new Vector3(400f, 0f, 0f);
            // Look back along -Z
            camera.LookAt(new Vector3(-1f, 0f, 0f));
            camera.NearClipDistance = 5;

            viewport = window.AddViewport(camera);
            viewport.BackgroundColour = new ColourValue(0.0f, 0.0f, 0.0f, 1.0f);



            material = MaterialManager.Singleton.Create("front", "General");
            material.GetTechnique(0).GetPass(0).CreateTextureUnitState("snow3.jpg");
            material = MaterialManager.Singleton.Create("other", "General");
            material.GetTechnique(0).GetPass(0).CreateTextureUnitState("snow.jpg");
            material = MaterialManager.Singleton.Create("back", "General");
            material.GetTechnique(0).GetPass(0).CreateTextureUnitState("back2.jpg");

            Vector3 moveVector3 = new Vector3(0f,-60f,110f);
            for (int i = 0; i < cubeCount; i++)
            {
                Cube cube = new Cube();
                cube.CubeName = "cube" + i.ToString();
                cube.BuildCube("front", ref sceneMgr);                

                ManualObject manual = sceneMgr.GetManualObject(cube.CubeName);
                manual.ConvertToMesh("cubeMesh"+i.ToString());
                Entity ent = sceneMgr.CreateEntity("box"+i.ToString(), "cubeMesh"+i.ToString());
                SceneNode node = sceneMgr.RootSceneNode.CreateChildSceneNode("boxNode"+i.ToString());
                ent.CastShadows = true;
                node.AttachObject(ent);
                float y = i * (Cube.cubeHeight + 10) + moveVector3.y;
                node.Position = new Vector3(0f, y, moveVector3.z);
            }

            Plane plane = new Plane();
            plane.BuildPlane("back",ref sceneMgr);
            ManualObject manualPlane = sceneMgr.GetManualObject(plane.PlaneName);
            manualPlane.ConvertToMesh("planeMesh");
            Entity planeEntity = sceneMgr.CreateEntity("planeEntity", "planeMesh");
            SceneNode planeNode = sceneMgr.RootSceneNode.CreateChildSceneNode("planeNode");
            planeNode.AttachObject(planeEntity);
            
        }

        public SceneNode GetScenceNode(int index)
        {
            SceneNode node = sceneMgr.GetSceneNode("boxNode"+index.ToString());
            return node;
        }

        public int GetCubeCount()
        {
            return cubeCount;
        }

        public void Paint()
        {
            root.RenderOneFrame();
        }

        public void Dispose()
        {
            if (material != null)
            {
                material.Dispose();
                material = null;
            }
            if (root != null)
            {
                root.Dispose();
                root = null;
            }            
        }
    }
}
