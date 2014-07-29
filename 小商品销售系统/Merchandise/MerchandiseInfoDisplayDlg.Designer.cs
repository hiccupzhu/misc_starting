namespace Merchandise
{
    partial class MerchandiseInfoDisplayDlg
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
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.exportBtn = new System.Windows.Forms.Button();
            this.queryBtn = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.existingCondition = new System.Windows.Forms.ComboBox();
            this.stockCondition = new System.Windows.Forms.ComboBox();
            this.existingCountsText = new System.Windows.Forms.TextBox();
            this.stockCountsText = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
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
            this.splitContainer1.Panel1.Controls.Add(this.groupBox2);
            this.splitContainer1.Panel1.Controls.Add(this.groupBox1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.dataGridView1);
            this.splitContainer1.Size = new System.Drawing.Size(677, 416);
            this.splitContainer1.SplitterDistance = 107;
            this.splitContainer1.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.exportBtn);
            this.groupBox2.Controls.Add(this.queryBtn);
            this.groupBox2.Location = new System.Drawing.Point(477, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(188, 92);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "操作";
            // 
            // exportBtn
            // 
            this.exportBtn.Location = new System.Drawing.Point(103, 36);
            this.exportBtn.Name = "exportBtn";
            this.exportBtn.Size = new System.Drawing.Size(67, 32);
            this.exportBtn.TabIndex = 0;
            this.exportBtn.Text = "导出查询结果";
            this.exportBtn.UseVisualStyleBackColor = true;
            this.exportBtn.Click += new System.EventHandler(this.exportBtn_Click);
            // 
            // queryBtn
            // 
            this.queryBtn.Location = new System.Drawing.Point(21, 36);
            this.queryBtn.Name = "queryBtn";
            this.queryBtn.Size = new System.Drawing.Size(67, 32);
            this.queryBtn.TabIndex = 0;
            this.queryBtn.Text = "查询";
            this.queryBtn.UseVisualStyleBackColor = true;
            this.queryBtn.Click += new System.EventHandler(this.queryBtn_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.existingCondition);
            this.groupBox1.Controls.Add(this.stockCondition);
            this.groupBox1.Controls.Add(this.existingCountsText);
            this.groupBox1.Controls.Add(this.stockCountsText);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.comboBox1);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(450, 92);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "查询条件";
            // 
            // existingCondition
            // 
            this.existingCondition.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.existingCondition.FormattingEnabled = true;
            this.existingCondition.Location = new System.Drawing.Point(278, 53);
            this.existingCondition.Name = "existingCondition";
            this.existingCondition.Size = new System.Drawing.Size(56, 20);
            this.existingCondition.TabIndex = 4;
            // 
            // stockCondition
            // 
            this.stockCondition.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.stockCondition.FormattingEnabled = true;
            this.stockCondition.Location = new System.Drawing.Point(82, 54);
            this.stockCondition.Name = "stockCondition";
            this.stockCondition.Size = new System.Drawing.Size(56, 20);
            this.stockCondition.TabIndex = 4;
            // 
            // existingCountsText
            // 
            this.existingCountsText.Location = new System.Drawing.Point(332, 53);
            this.existingCountsText.Name = "existingCountsText";
            this.existingCountsText.Size = new System.Drawing.Size(68, 21);
            this.existingCountsText.TabIndex = 3;
            // 
            // stockCountsText
            // 
            this.stockCountsText.Location = new System.Drawing.Point(138, 54);
            this.stockCountsText.Name = "stockCountsText";
            this.stockCountsText.Size = new System.Drawing.Size(65, 21);
            this.stockCountsText.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(243, 56);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(29, 12);
            this.label3.TabIndex = 2;
            this.label3.Text = "现存";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(47, 56);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(29, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "库存";
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(83, 21);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(317, 20);
            this.comboBox1.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(23, 27);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "商品名称";
            // 
            // dataGridView1
            // 
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dataGridView1.Location = new System.Drawing.Point(0, 0);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowTemplate.Height = 23;
            this.dataGridView1.Size = new System.Drawing.Size(677, 305);
            this.dataGridView1.TabIndex = 0;
            // 
            // MerchandiseInfoDisplayDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(677, 416);
            this.Controls.Add(this.splitContainer1);
            this.MaximizeBox = false;
            this.Name = "MerchandiseInfoDisplayDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "商品信息查询窗口";
            this.Load += new System.EventHandler(this.MerchandiseInfoDisplayDlg_Load);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button exportBtn;
        private System.Windows.Forms.Button queryBtn;
        private System.Windows.Forms.TextBox existingCountsText;
        private System.Windows.Forms.TextBox stockCountsText;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox existingCondition;
        private System.Windows.Forms.ComboBox stockCondition;
    }
}