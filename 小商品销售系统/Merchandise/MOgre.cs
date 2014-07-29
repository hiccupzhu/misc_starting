using System;
using System.Collections.Generic;
using System.Text;
using Mogre;
using System.Drawing;

namespace Merchandise
{
    public class OgreWindow
    {
        private int cubeCount = 5;
        private MaterialPtr material = null;
        public Root root;
        public SceneManager sceneMgr;

        protected Camera camera = null;
        protected Viewport viewport = null;
        protected RenderWindow window = null;
        protected RaySceneQuery raySceneQuery = null;
        protected ParticleSystem particleSystem = null;
        protected IntPtr hWnd;

        private bool isTubeRool = true;

        public int CubeCount { get { return cubeCount; } set { cubeCount = value; } }

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

            // Create a point light
            Light l = sceneMgr.CreateLight("MainLight");
            l.Position = new Vector3(500, 500, 500);

            //----------------------------------------------------- 
            // 5 Create the camera 
            //----------------------------------------------------- 
            camera = sceneMgr.CreateCamera("SimpleCamera");
            camera.Position = new Vector3(400f, 0f, 0f);//400
            // Look back along -Z
            camera.LookAt(new Vector3(-1f, 0f, 0f));
            camera.NearClipDistance = 5;

            viewport = window.AddViewport(camera);
            viewport.BackgroundColour = new ColourValue(0.0f, 0.0f, 0.0f, 1.0f);

            for (int i = 0; i < cubeCount; i++)
            {
                material = MaterialManager.Singleton.Create("front" + i.ToString(), "General");
                material.GetTechnique(0).GetPass(0).CreateTextureUnitState((cubeCount - 1 - i).ToString() + ".jpg");
            }
            material = MaterialManager.Singleton.Create("other", "General");
            material.GetTechnique(0).GetPass(0).CreateTextureUnitState("snow.jpg");
            material = MaterialManager.Singleton.Create("back", "General");
            material.GetTechnique(0).GetPass(0).CreateTextureUnitState("back2.jpg");

            Vector3 moveVector3 = new Vector3(0f,-60f,110f);
            for (int i = 0; i < cubeCount; i++)
            {
                Cube cube = new Cube();
                cube.CubeName = "cube" + i.ToString();
                cube.BuildCube("front" + i.ToString(), ref sceneMgr);              

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


            //载入tube1-4
            SceneNode tubeRootNode = sceneMgr.RootSceneNode.CreateChildSceneNode("tubeRoot");
            tubeRootNode.Position = new Vector3(0f,-110f,-170f);
            for (int i = 1; i <= 4;i++ )
            {
                string tubeName = "tube" + i.ToString();
                Entity tubeEnt = sceneMgr.CreateEntity(tubeName, tubeName + ".mesh");
                switch  (i)
                {
                    case 1:
                        tubeEnt.SetMaterialName("Examples/TextureEffect1");
                        break;
                    case 2:
                        tubeEnt.SetMaterialName("Examples/TextureEffect4");
                        break;
                    case 3:
                        tubeEnt.SetMaterialName("Examples/TextureEffect3");
                        break;
                    case 4:
                        tubeEnt.SetMaterialName("Examples/TextureEffect2");
                        break;
                }
                
                SceneNode tubeNode = tubeRootNode.CreateChildSceneNode(tubeName + "Node");
                tubeNode.AttachObject(tubeEnt);
            }
            tubeRootNode.Scale(new Vector3(0.3f, 0.3f, 0.3f));


            raySceneQuery = sceneMgr.CreateRayQuery(new Ray());

            particleSystem = sceneMgr.CreateParticleSystem("partical1", "Example/Snow");
            sceneMgr.RootSceneNode.CreateChildSceneNode().AttachObject(particleSystem);

            
            
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
            this.TubeRoll();
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
                sceneMgr.DestroyQuery(raySceneQuery);
                raySceneQuery = null;
                root.Dispose();
                root = null;
                camera = null;
                sceneMgr = null;
                viewport = null;
                window = null;
                hWnd = (IntPtr)0;
            }            
        }

        public string selectObject(Point mousePoint,float width,float height)
        {
            RestoreBoxnodes();

            Ray ray = camera.GetCameraToViewportRay(mousePoint.X / width, mousePoint.Y / height);
            raySceneQuery.Ray = ray;

            // Execute query   
            raySceneQuery.SetSortByDistance(true, 1);
            RaySceneQueryResult result = raySceneQuery.Execute();
            if (result.Count > 0)
            {
                isTubeRool = true;
                RaySceneQueryResultEntry entry = result[0];
                string entryName = entry.movable.Name;
                if (entryName.Substring(0, 3).Equals("box"))
                {
                    Node node = entry.movable.ParentNode;
                    Vector3 nodePosition = node.Position;
                    if(nodePosition.x == 0)
                    {
                        nodePosition.x += 20;
                        node.Position = nodePosition;
                    }
                }
                else if (entryName.Substring(0, 4).Equals("tube"))
                {
                    isTubeRool = false;
                    Node nodeParent = entry.movable.ParentNode.Parent;
                    for (int i = 1; i < 4; i++)
                    {
                        Node nodeChild = nodeParent.GetChild("tube" + i.ToString() + "Node");
                        nodeChild.ResetOrientation();
                    }
                }
                return entryName;
            }
            return "";
        }

        public void TubeRoll()
        {
            if (isTubeRool)
            {
                Node node = sceneMgr.GetSceneNode("tubeRoot");
                for (int i = 1; i <= 4; i++)
                {
                    Node nodeChild = node.GetChild("tube" + i.ToString() + "Node");
                    switch (i)
                    {
                        case 1:
                            nodeChild.Roll(0.03f);
                            break;
                        case 2:
                            nodeChild.Pitch(0.05f);
                            nodeChild.Roll(0.1f);
                            break;
                        case 3:
                            nodeChild.Yaw(0.08f);
                            nodeChild.Pitch(0.03f);
                            break;
                        case 4:
                            break;
                    }
                }
            }
        }

        public void RestoreBoxnodes()
        {
            for(int i=0;i<cubeCount;i++)
            {
                Node node = sceneMgr.GetSceneNode("boxNode"+i.ToString());
                Vector3 nodePosition = node.Position;
                if (nodePosition.x > 0)
                {
                    nodePosition.x = 0;
                    node.Position = nodePosition;
                }
                
            }
        }
    }
}
