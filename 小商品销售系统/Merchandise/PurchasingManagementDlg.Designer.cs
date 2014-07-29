namespace Merchandise
{
    partial class PurchasingManagementDlg
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
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ExtingCountText = new System.Windows.Forms.TextBox();
            this.ExtingCountLable = new System.Windows.Forms.Label();
            this.Exit = new System.Windows.Forms.Button();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.AddMerchandise = new System.Windows.Forms.Button();
            this.listBox1 = new System.Windows.Forms.ListBox();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // dataGridView1
            // 
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Location = new System.Drawing.Point(23, 12);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowTemplate.Height = 23;
            this.dataGridView1.Size = new System.Drawing.Size(644, 177);
            this.dataGridView1.TabIndex = 0;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ExtingCountText);
            this.groupBox1.Controls.Add(this.ExtingCountLable);
            this.groupBox1.Controls.Add(this.Exit);
            this.groupBox1.Controls.Add(this.textBox2);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.AddMerchandise);
            this.groupBox1.Controls.Add(this.listBox1);
            this.groupBox1.Location = new System.Drawing.Point(12, 202);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(673, 223);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "操作";
            // 
            // ExtingCountText
            // 
            this.ExtingCountText.Location = new System.Drawing.Point(468, 84);
            this.ExtingCountText.Name = "ExtingCountText";
            this.ExtingCountText.Size = new System.Drawing.Size(100, 21);
            this.ExtingCountText.TabIndex = 8;
            this.ExtingCountText.Visible = false;
            // 
            // ExtingCountLable
            // 
            this.ExtingCountLable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ExtingCountLable.AutoSize = true;
            this.ExtingCountLable.Location = new System.Drawing.Point(409, 87);
            this.ExtingCountLable.Name = "ExtingCountLable";
            this.ExtingCountLable.Size = new System.Drawing.Size(53, 12);
            this.ExtingCountLable.TabIndex = 7;
            this.ExtingCountLable.Text = "现    存";
            this.ExtingCountLable.Visible = false;
            // 
            // Exit
            // 
            this.Exit.Location = new System.Drawing.Point(542, 139);
            this.Exit.Name = "Exit";
            this.Exit.Size = new System.Drawing.Size(75, 23);
            this.Exit.TabIndex = 4;
            this.Exit.Text = "退出";
            this.Exit.UseVisualStyleBackColor = true;
            this.Exit.Click += new System.EventHandler(this.Exit_Click);
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(468, 45);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(100, 21);
            this.textBox2.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(409, 48);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "进货数量";
            // 
            // AddMerchandise
            // 
            this.AddMerchandise.Location = new System.Drawing.Point(411, 139);
            this.AddMerchandise.Name = "AddMerchandise";
            this.AddMerchandise.Size = new System.Drawing.Size(75, 23);
            this.AddMerchandise.TabIndex = 1;
            this.AddMerchandise.Text = "添加";
            this.AddMerchandise.UseVisualStyleBackColor = true;
            this.AddMerchandise.Click += new System.EventHandler(this.AddMerchandise_Click);
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.ItemHeight = 12;
            this.listBox1.Location = new System.Drawing.Point(11, 19);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(345, 184);
            this.listBox1.TabIndex = 0;
            // 
            // PurchasingManagementDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(692, 436);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.dataGridView1);
            this.MaximizeBox = false;
            this.Name = "PurchasingManagementDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "商品进货管理窗口";
            this.Load += new System.EventHandler(this.PurchasingManagementDlg_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Button Exit;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button AddMerchandise;
        private System.Windows.Forms.Label ExtingCountLable;
        private System.Windows.Forms.TextBox ExtingCountText;

    }
}