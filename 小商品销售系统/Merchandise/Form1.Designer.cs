namespace Merchandise
{
    partial class Form1
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.操作OToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MerchandiseEntering = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.PurchasingManagement = new System.Windows.Forms.ToolStripMenuItem();
            this.SalesManagement = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.Exit = new System.Windows.Forms.ToolStripMenuItem();
            this.查询SToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MerchandiseInfoDisplay = new System.Windows.Forms.ToolStripMenuItem();
            this.SaleRecordQuery = new System.Windows.Forms.ToolStripMenuItem();
            this.帮助HToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.关于AToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.panel1 = new System.Windows.Forms.Panel();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.xPosition = new System.Windows.Forms.ToolStripStatusLabel();
            this.yPosition = new System.Windows.Forms.ToolStripStatusLabel();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.stockRecordQuery = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.操作OToolStripMenuItem,
            this.查询SToolStripMenuItem,
            this.帮助HToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(727, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // 操作OToolStripMenuItem
            // 
            this.操作OToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MerchandiseEntering,
            this.toolStripMenuItem1,
            this.PurchasingManagement,
            this.SalesManagement,
            this.toolStripMenuItem2,
            this.Exit});
            this.操作OToolStripMenuItem.Name = "操作OToolStripMenuItem";
            this.操作OToolStripMenuItem.Size = new System.Drawing.Size(59, 20);
            this.操作OToolStripMenuItem.Text = "操作(&O)";
            // 
            // MerchandiseEntering
            // 
            this.MerchandiseEntering.Name = "MerchandiseEntering";
            this.MerchandiseEntering.Size = new System.Drawing.Size(142, 22);
            this.MerchandiseEntering.Text = "产品录入";
            this.MerchandiseEntering.Click += new System.EventHandler(this.MerchandiseEntering_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(139, 6);
            // 
            // PurchasingManagement
            // 
            this.PurchasingManagement.Name = "PurchasingManagement";
            this.PurchasingManagement.Size = new System.Drawing.Size(142, 22);
            this.PurchasingManagement.Text = "商品进货管理";
            this.PurchasingManagement.Click += new System.EventHandler(this.PurchasingManagement_Click);
            // 
            // SalesManagement
            // 
            this.SalesManagement.Name = "SalesManagement";
            this.SalesManagement.Size = new System.Drawing.Size(142, 22);
            this.SalesManagement.Text = "商品销售管理";
            this.SalesManagement.Click += new System.EventHandler(this.SalesManagement_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(139, 6);
            // 
            // Exit
            // 
            this.Exit.Name = "Exit";
            this.Exit.Size = new System.Drawing.Size(142, 22);
            this.Exit.Text = "退出(&X)";
            this.Exit.Click += new System.EventHandler(this.Exit_Click);
            // 
            // 查询SToolStripMenuItem
            // 
            this.查询SToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MerchandiseInfoDisplay,
            this.SaleRecordQuery,
            this.stockRecordQuery});
            this.查询SToolStripMenuItem.Name = "查询SToolStripMenuItem";
            this.查询SToolStripMenuItem.Size = new System.Drawing.Size(59, 20);
            this.查询SToolStripMenuItem.Text = "查询(&S)";
            // 
            // MerchandiseInfoDisplay
            // 
            this.MerchandiseInfoDisplay.Name = "MerchandiseInfoDisplay";
            this.MerchandiseInfoDisplay.Size = new System.Drawing.Size(166, 22);
            this.MerchandiseInfoDisplay.Text = "商品信息查询";
            this.MerchandiseInfoDisplay.Click += new System.EventHandler(this.MerchandiseInfoDisplay_Click);
            // 
            // SaleRecordQuery
            // 
            this.SaleRecordQuery.Name = "SaleRecordQuery";
            this.SaleRecordQuery.Size = new System.Drawing.Size(166, 22);
            this.SaleRecordQuery.Text = "商品出售记录查询";
            this.SaleRecordQuery.Click += new System.EventHandler(this.SaleRecordQuery_Click);
            // 
            // 帮助HToolStripMenuItem
            // 
            this.帮助HToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.关于AToolStripMenuItem});
            this.帮助HToolStripMenuItem.Name = "帮助HToolStripMenuItem";
            this.帮助HToolStripMenuItem.Size = new System.Drawing.Size(59, 20);
            this.帮助HToolStripMenuItem.Text = "帮助(&H)";
            // 
            // 关于AToolStripMenuItem
            // 
            this.关于AToolStripMenuItem.Name = "关于AToolStripMenuItem";
            this.关于AToolStripMenuItem.Size = new System.Drawing.Size(112, 22);
            this.关于AToolStripMenuItem.Text = "关于(&A)";
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.statusStrip1);
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 24);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(727, 366);
            this.panel1.TabIndex = 2;
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.xPosition,
            this.yPosition});
            this.statusStrip1.Location = new System.Drawing.Point(0, 344);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(727, 22);
            this.statusStrip1.TabIndex = 3;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // xPosition
            // 
            this.xPosition.Name = "xPosition";
            this.xPosition.Size = new System.Drawing.Size(23, 17);
            this.xPosition.Text = "X:0";
            // 
            // yPosition
            // 
            this.yPosition.Name = "yPosition";
            this.yPosition.Size = new System.Drawing.Size(23, 17);
            this.yPosition.Text = "Y:0";
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.pictureBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBox1.Location = new System.Drawing.Point(0, 0);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(727, 366);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 2;
            this.pictureBox1.TabStop = false;
            // 
            // stockRecordQuery
            // 
            this.stockRecordQuery.Name = "stockRecordQuery";
            this.stockRecordQuery.Size = new System.Drawing.Size(166, 22);
            this.stockRecordQuery.Text = "商品进货记录查询";
            this.stockRecordQuery.Click += new System.EventHandler(this.stockRecordQuery_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(727, 390);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "货物管理主窗口";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem 操作OToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem PurchasingManagement;
        private System.Windows.Forms.ToolStripMenuItem SalesManagement;
        private System.Windows.Forms.ToolStripMenuItem 查询SToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem MerchandiseInfoDisplay;
        private System.Windows.Forms.ToolStripMenuItem 帮助HToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 关于AToolStripMenuItem;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.ToolStripMenuItem MerchandiseEntering;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem Exit;
        private System.Windows.Forms.ToolStripMenuItem SaleRecordQuery;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel xPosition;
        private System.Windows.Forms.ToolStripStatusLabel yPosition;
        private System.Windows.Forms.ToolStripMenuItem stockRecordQuery;
    }
}

