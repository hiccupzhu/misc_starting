using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
//using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Mogre;
using System.Threading;
using System.Runtime.InteropServices;

namespace MOgreTest
{
    public partial class Form1 : Form
    {
        protected OgreWindow mogreWin = null;
        private SceneNode node = null;
        private Thread threadWork = null;
        private volatile bool _threadAlive = false;

        public Form1()
        {
            InitializeComponent();       
            
            mogreWin = new OgreWindow(panel1.Handle);
            mogreWin.InitMogre();
            node = mogreWin.GetScenceNode(0);            
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.Disposed += new EventHandler(Form1_Disposed);
            startThread();
        }

        void Form1_Disposed(object sender, EventArgs e)
        {
            if (_threadAlive)
            {
                _threadAlive = false;
                if(!threadWork.Join(3000))
                {
                    MessageBox.Show("3D线程销毁失败");
                }                
            }
            mogreWin.Dispose();
        }

        private void startBtn_Click(object sender, EventArgs e)
        {
            startThread();
        }

        public void startThread()
        {
            try
            {
                if (!threadWork.IsAlive)
                {
                    _threadAlive = true;
                    threadWork.Start("hello");

                    while (!threadWork.IsAlive) ;
                }
            }
            catch (System.Exception ex)
            {
                _threadAlive = true;
                threadWork = new Thread(new ParameterizedThreadStart(thread_Run));
                threadWork.Start("hello");
                Trace.WriteLine(ex.ToString());
            }
        }

        private void stopBtn_Click(object sender, EventArgs e)
        {
            if(threadWork.IsAlive)
            {
                _threadAlive = false;
                
                if (!threadWork.Join(3000))
                {
                    MessageBox.Show("3D线程销毁失败");
                }
            }
        }

        public void DoSomething()
        {
            node.Roll(-0.1f);
            mogreWin.Paint();
        }

        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceCounter(
            out long lpPerformanceCount);

        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceFrequency(
            out long lpFrequency);

        public void thread_Run(Object param)
        {
            long curTime = 0;       // current time
            long timeCount = 40;    // ms per frame, default if no performance counter
            long perfFrequency=0;       // performance timer frequency
            long frameFraquency = 30;
            bool perfFlag = false;    // flag determining which timer to use
            long nextTime = 0;    // time to render next frame

            if (QueryPerformanceFrequency(out perfFrequency))
            {
                perfFlag = true;
                timeCount = perfFrequency / frameFraquency;
                QueryPerformanceCounter(out nextTime);
            }
            else
            {
                nextTime = System.Environment.TickCount;
            }

            while(_threadAlive)
            {                
                if (perfFlag)
                    QueryPerformanceCounter(out curTime);
                else
                    curTime = System.Environment.TickCount;
                
                if(curTime>nextTime)
                {
                    DoSomething();

                    nextTime += timeCount;
                    if(nextTime<curTime)
                    {
                        nextTime = curTime + timeCount;
                    }
                }                
            }
            Trace.WriteLine("thread exit");
        }

        

        protected override void WndProc(ref System.Windows.Forms.Message e)
        {
            if (e.Msg == 0x10)
                _threadAlive = false;

            base.WndProc(ref e);
        } 

    }
}
