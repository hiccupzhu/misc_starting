using System;
using System.Collections.Generic;
using System.Text;
using Mogre;

namespace MOgreTest
{
    class Plane
    {
        public int planeWidth = 500;
        public int planeHeight = 400;

        private string planeName = "plane";
        public string PlaneName { get { return planeName; } set { planeName = value; } }

        public void BuildPlane(string materialName,ref SceneManager mSceneMgr)
        {
            float z = planeWidth / 2;
            float y = planeHeight / 2;
            float x = -40f;

            ManualObject cube = mSceneMgr.CreateManualObject(planeName);

            cube.Begin(materialName, RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(x, -y, -z);
            cube.TextureCoord(1, 1);
            cube.Position(x, y, -z);
            cube.TextureCoord(1, 0);
            cube.Position(x, y, z);
            cube.TextureCoord(0, 0);
            cube.Position(x, -y, z);
            cube.TextureCoord(0, 1);                
            
// 
//             cube.Triangle(0, 2, 1);
//             cube.Triangle(0, 2, 3); 
            cube.End();
        }
    }
}
