namespace Merchandise
{
    partial class MerchandiseEnteringDlg
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.costPriceText = new System.Windows.Forms.TextBox();
            this.extingCountsText = new System.Windows.Forms.TextBox();
            this.stockCountsText = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.MerchandiseName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.ExitBtn = new System.Windows.Forms.Button();
            this.DeleteBtn = new System.Windows.Forms.Button();
            this.EnteringBtn = new System.Windows.Forms.Button();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.dataGridView1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.groupBox1);
            this.splitContainer1.Panel2.Controls.Add(this.ExitBtn);
            this.splitContainer1.Panel2.Controls.Add(this.DeleteBtn);
            this.splitContainer1.Panel2.Controls.Add(this.EnteringBtn);
            this.splitContainer1.Size = new System.Drawing.Size(732, 396);
            this.splitContainer1.SplitterDistance = 238;
            this.splitContainer1.TabIndex = 0;
            // 
            // dataGridView1
            // 
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dataGridView1.Location = new System.Drawing.Point(0, 0);
            this.dataGridView1.MultiSelect = false;
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.RowTemplate.Height = 23;
            this.dataGridView1.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridView1.Size = new System.Drawing.Size(732, 238);
            this.dataGridView1.TabIndex = 0;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.costPriceText);
            this.groupBox1.Controls.Add(this.extingCountsText);
            this.groupBox1.Controls.Add(this.stockCountsText);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.MerchandiseName);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(37, 6);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(330, 135);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "商品基本信息";
            // 
            // costPriceText
            // 
            this.costPriceText.Location = new System.Drawing.Point(91, 101);
            this.costPriceText.Name = "costPriceText";
            this.costPriceText.Size = new System.Drawing.Size(100, 21);
            this.costPriceText.TabIndex = 3;
            // 
            // extingCountsText
            // 
            this.extingCountsText.Location = new System.Drawing.Point(91, 74);
            this.extingCountsText.Name = "extingCountsText";
            this.extingCountsText.Size = new System.Drawing.Size(100, 21);
            this.extingCountsText.TabIndex = 3;
            // 
            // stockCountsText
            // 
            this.stockCountsText.Location = new System.Drawing.Point(91, 47);
            this.stockCountsText.Name = "stockCountsText";
            this.stockCountsText.Size = new System.Drawing.Size(100, 21);
            this.stockCountsText.TabIndex = 2;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(44, 110);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(41, 12);
            this.label4.TabIndex = 0;
            this.label4.Text = "成本价";
            // 
            // MerchandiseName
            // 
            this.MerchandiseName.Location = new System.Drawing.Point(91, 20);
            this.MerchandiseName.Name = "MerchandiseName";
            this.MerchandiseName.Size = new System.Drawing.Size(192, 21);
            this.MerchandiseName.TabIndex = 1;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(56, 83);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(29, 12);
            this.label3.TabIndex = 0;
            this.label3.Text = "现存";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(56, 56);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(29, 12);
            this.label2.TabIndex = 0;
            this.label2.Text = "库存";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(32, 29);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "商品名称";
            // 
            // ExitBtn
            // 
            this.ExitBtn.Location = new System.Drawing.Point(485, 106);
            this.ExitBtn.Name = "ExitBtn";
            this.ExitBtn.Size = new System.Drawing.Size(90, 23);
            this.ExitBtn.TabIndex = 5;
            this.ExitBtn.Text = "退出";
            this.ExitBtn.UseVisualStyleBackColor = true;
            this.ExitBtn.Click += new System.EventHandler(this.ExitBtn_Click);
            // 
            // DeleteBtn
            // 
            this.DeleteBtn.Location = new System.Drawing.Point(485, 24);
            this.DeleteBtn.Name = "DeleteBtn";
            this.DeleteBtn.Size = new System.Drawing.Size(90, 23);
            this.DeleteBtn.TabIndex = 4;
            this.DeleteBtn.Text = "删除";
            this.DeleteBtn.UseVisualStyleBackColor = true;
            this.DeleteBtn.Click += new System.EventHandler(this.DeleteBtn_Click);
            // 
            // EnteringBtn
            // 
            this.EnteringBtn.Location = new System.Drawing.Point(485, 65);
            this.EnteringBtn.Name = "EnteringBtn";
            this.EnteringBtn.Size = new System.Drawing.Size(90, 23);
            this.EnteringBtn.TabIndex = 4;
            this.EnteringBtn.Text = "录入";
            this.EnteringBtn.UseVisualStyleBackColor = true;
            this.EnteringBtn.Click += new System.EventHandler(this.EnteringBtn_Click);
            // 
            // MerchandiseEnteringDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(732, 396);
            this.Controls.Add(this.splitContainer1);
            this.MaximizeBox = false;
            this.Name = "MerchandiseEnteringDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "商品录入窗口";
            this.Load += new System.EventHandler(this.MerchandiseEnteringDlg_Load_1);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.TextBox extingCountsText;
        private System.Windows.Forms.TextBox stockCountsText;
        private System.Windows.Forms.TextBox MerchandiseName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button ExitBtn;
        private System.Windows.Forms.Button EnteringBtn;
        private System.Windows.Forms.Button DeleteBtn;
        private System.Windows.Forms.TextBox costPriceText;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
    }
}