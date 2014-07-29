using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using Mogre;
using System.Runtime.InteropServices;

namespace Merchandise
{
    public delegate void OpentionEvent(Object sender,EventArgs e);
    public partial class Form1 : Form
    {
        protected OgreWindow mogreWin = null;
        private Thread threadWork = null;
        private volatile bool _threadAlive = false;

        private PurchasingManagementDlg purchasingManagementDlg;
        private MerchandiseEnteringDlg merchandiseEnteringDlg;
        private SalesMenagementDlg salesMenagementDlg;
        private MerchandiseInfoDisplayDlg merchandiseInfoDisplayDlg;
        private SaleRecordQueryDlg saleRecordQueryDlg;
        private StockRecordQuery stockRecordQueryDlg;
        private OpentionEvent[] opentionEvent=new OpentionEvent[5];

        public Form1(string[] args)
        {
            InitializeComponent();
            this.Width = 800;
            this.Height = 600;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.FormClosed +=new FormClosedEventHandler(Form1_FormClosed);
            if (args.Length > 0)
            {
                this.Text = args[0] + "管理窗体";
            }

            mogreWin = new OgreWindow(pictureBox1.Handle);
            mogreWin.InitMogre();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.pictureBox1.MouseMove += new MouseEventHandler(pictureBox1_MouseMove);
            this.pictureBox1.MouseClick +=new MouseEventHandler(pictureBox1_MouseClick);
            this.Disposed += new EventHandler(Form1_Disposed);
            this.ResizeBegin +=new EventHandler(Form1_ResizeBegin);
            this.ResizeEnd +=new EventHandler(Form1_ResizeEnd);
            this.Resize += new EventHandler(Form1_Resize);

            
            opentionEvent[0] = MerchandiseEntering_Click;
            opentionEvent[1] = PurchasingManagement_Click;
            opentionEvent[2] = SalesManagement_Click;
            opentionEvent[3] = MerchandiseInfoDisplay_Click;
            opentionEvent[4] = SaleRecordQuery_Click;
            
            startThread();
        }

        void Form1_Disposed(object sender, EventArgs e)
        {
            stopThread();
            mogreWin.Dispose();
        }

        public void Form1_ResizeBegin(Object sender, EventArgs e)
        {
            stopThread();
        }

        public void Form1_Resize(Object sender,EventArgs e)
        {
            stopThread();
            startThread();
        }

        public void Form1_ResizeEnd(Object sender, EventArgs e)
        {
            startThread();
        }

        public void Form1_FormClosed(Object sender,EventArgs e)
        {
            MyDataBase.CloseConn();
        }

        private void PurchasingManagement_Click(object sender, EventArgs e)
        {
            try
            {
                purchasingManagementDlg.Visible = true;
                purchasingManagementDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                purchasingManagementDlg = new PurchasingManagementDlg();
                purchasingManagementDlg.Show();
            }
                
        }

        private void MerchandiseInfoDisplay_Click(object sender, EventArgs e)
        {
            try
            {
                merchandiseInfoDisplayDlg.Visible = true;
                merchandiseInfoDisplayDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                merchandiseInfoDisplayDlg = new MerchandiseInfoDisplayDlg();
                merchandiseInfoDisplayDlg.Show();
            }
        }

        

        public void pictureBox1_MouseMove(Object sender, MouseEventArgs e)
        {
            xPosition.Text = "X:" + e.Y.ToString();
            yPosition.Text = "Y:" + e.Y.ToString();

            if(_threadAlive)
            {
                Point point =new Point(e.X,e.Y);
                mogreWin.selectObject(point,pictureBox1.Width,pictureBox1.Height);
                
            }
        }

        public void pictureBox1_MouseClick(Object sender,MouseEventArgs e)
        {
            if(_threadAlive)
            {
                Point point = new Point(e.X, e.Y);
                string objName =mogreWin.selectObject(point, pictureBox1.Width, pictureBox1.Height);
                if (objName.Substring(0, 3).Equals("box"))
                {
                    int boxNum = System.Convert.ToInt32(objName.Substring(3, 1));
                    opentionEvent[4 - boxNum](sender, e);
                }
                else if (objName.Substring(0, 4).Equals("tube"))
                {
                    this.Close();
                }
                Trace.WriteLine(e.Button.ToString() + " X=" + e.X + " Y=" + e.Y.ToString());
            }
        }

        private void MerchandiseEntering_Click(object sender, EventArgs e)
        {
            try
            {
                merchandiseEnteringDlg.Visible = true;
                merchandiseEnteringDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                merchandiseEnteringDlg = new MerchandiseEnteringDlg();
                merchandiseEnteringDlg.Show();
            }
            
        }

        private void Exit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void SalesManagement_Click(object sender, EventArgs e)
        {
            try
            {
                salesMenagementDlg.Visible = true;
                salesMenagementDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                salesMenagementDlg = new SalesMenagementDlg();
                salesMenagementDlg.Show();
            }
            
        }

        private void testToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void SaleRecordQuery_Click(object sender, EventArgs e)
        {
            try
            {
                saleRecordQueryDlg.Visible = true;
                saleRecordQueryDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                saleRecordQueryDlg = new SaleRecordQueryDlg();
                saleRecordQueryDlg.Show();
            }
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

        public void stopThread()
        {
            if (_threadAlive)
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
            long perfFrequency = 0;       // performance timer frequency
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

            while (_threadAlive)
            {
                if (perfFlag)
                    QueryPerformanceCounter(out curTime);
                else
                    curTime = System.Environment.TickCount;

                if (curTime > nextTime)
                {
                    DoSomething();

                    nextTime += timeCount;
                    if (nextTime < curTime)
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

        private void stockRecordQuery_Click(object sender, EventArgs e)
        {
            try
            {
                stockRecordQueryDlg.Visible = true;
                stockRecordQueryDlg.Focus();
            }
            catch (System.Exception ex)
            {
                Trace.WriteLine(ex.ToString());
                stockRecordQueryDlg = new StockRecordQuery();
                stockRecordQueryDlg.Show();
            }
        } 

    }
}
