using System;
using System.Collections.Generic;
using System.Text;
using Mogre;

namespace Merchandise
{
    public class Cube
    {
        public static int cubeWidth = 30;
        public static int cubeHeight = 30;
        public static int cubeLength = 80;

        private string cubeName = "cube";

        public string CubeName { get { return cubeName; } set { cubeName = value; } }

        public void BuildCube(string materialName,ref SceneManager mSceneMgr)
        {
            ManualObject cube = mSceneMgr.CreateManualObject(cubeName);
            int x = cubeHeight / 2;
            int y = cubeWidth / 2;
            int z = cubeLength / 2;

            cube.Begin("other", RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(-x, -y, -z);   //0 
            cube.TextureCoord(1, 0);
            cube.Position(x, -y, -z);    //1 
            cube.TextureCoord(0, 0);
            cube.Position(x, -y, z);    //2 
            cube.TextureCoord(0, 1);
            cube.Position(-x, -y, z);    //3 
            cube.TextureCoord(1, 1);
            cube.End();

            /// 左面 
            cube.Begin("other", RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(-x, -y, z);    //3 
            cube.TextureCoord(1, 0);
            cube.Position(-x, y, z);    //7 
            cube.TextureCoord(0, 0);
            cube.Position(-x, y, -z);    //4 
            cube.TextureCoord(0, 1);
            cube.Position(-x, -y, -z);   //0 
            cube.TextureCoord(1, 1);
            cube.End();

            /// 上面 
            cube.Begin("other", RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(-x, y, -z);    //4 
            cube.TextureCoord(1, 0);
            cube.Position(-x, y, z);    //7 
            cube.TextureCoord(0, 0);
            cube.Position(x, y, z);    //6 
            cube.TextureCoord(0, 1);
            cube.Position(x, y, -z);    //5 
            cube.TextureCoord(1, 1);
            cube.End();

            /// 右面 
            cube.Begin(materialName, RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(x, -y, -z);    //1 
            cube.TextureCoord(1, 1);
            cube.Position(x, y, -z);    //5 
            cube.TextureCoord(1, 0);
            cube.Position(x, y, z);    //6 
            cube.TextureCoord(0, 0);
            cube.Position(x, -y, z);    //2 
            cube.TextureCoord(0, 1);
            cube.End();

            /// 前面 
            cube.Begin("other", RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(x, -y, -z);    //1 
            cube.TextureCoord(1, 0);
            cube.Position(-x, -y, -z);   //0 
            cube.TextureCoord(0, 0);
            cube.Position(-x, y, -z);    //4 
            cube.TextureCoord(0, 1);
            cube.Position(x, y, -z);    //5 
            cube.TextureCoord(1, 1);
            cube.End();

            /// 后面 
            cube.Begin("other", RenderOperation.OperationTypes.OT_TRIANGLE_FAN);
            cube.Position(x, -y, z);    //2 
            cube.TextureCoord(1, 0);
            cube.Position(x, y, z);    //6 
            cube.TextureCoord(0, 0);
            cube.Position(-x, y, z);    //7 
            cube.TextureCoord(0, 1);
            cube.Position(-x, -y, z);    //3 
            cube.TextureCoord(1, 1);
            cube.End();

        }
    }
}
