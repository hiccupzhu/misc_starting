using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;

using System.Text;
using System.Windows.Forms;
using System.Data.OleDb;
using System.Diagnostics;

namespace Merchandise
{
    

    public partial class PurchasingManagementDlg : Form
    {
        private OleDbCommand sqlcmd = new OleDbCommand();

        public PurchasingManagementDlg()
        {
            InitializeComponent();
            listBox1.MouseClick += new MouseEventHandler(listBox1_MouseClick);
            dataGridView1.MouseClick += new MouseEventHandler(dataGridView1_MouseClick);
            dataGridView1.LostFocus +=new EventHandler(dataGridView1_LostFocus);
            textBox2.KeyPress += new KeyPressEventHandler(textBox_KeyPress);
            ExtingCountText.KeyPress += new KeyPressEventHandler(textBox_KeyPress);

            ExtingCountLable.Visible = false;
            ExtingCountText.Visible = false;            
        }

        public void textBox_KeyPress(Object sender, KeyPressEventArgs e)
        {
            if (!(e.KeyChar >= 48 && e.KeyChar <= 57 || e.KeyChar == 8))
            {
                e.Handled = true;
            }

        }

        public void listBox1_MouseClick(Object sender, MouseEventArgs e)
        {
            AddMerchandise.Text = "添加";
            ExtingCountLable.Visible = false;
            ExtingCountText.Visible = false;
        }

        public void dataGridView1_MouseClick(Object sender,MouseEventArgs e)
        {
            if (dataGridView1.SelectedRows.Count != 0)
            {
                if(dataGridView1.SelectedRows[0].Cells.Count>1)
                {
                    //MessageBox.Show(cellCount.ToString());
                    DataGridViewRow dRow = dataGridView1.SelectedRows[0];

                    for (int i = 0; i < listBox1.Items.Count; i++)
                    {
                        if (listBox1.Items[i].ToString().Equals(dRow.Cells[0].Value.ToString()))
                        {
                            listBox1.SelectedIndex = i;
                        }
                    }
                    textBox2.Text = dRow.Cells[1].Value.ToString();
                    ExtingCountText.Text = dRow.Cells[2].Value.ToString();

                    ExtingCountLable.Visible = true;
                    ExtingCountText.Visible = true;

                    AddMerchandise.Text = "更新";
                }               
            }
        }

        public void dataGridView1_LostFocus(Object sender, EventArgs e)
        {
            
        }

        private void PurchasingManagementDlg_Load(object sender, EventArgs e)
        {
            UpdateDataGridView();
            //商品名称列表
            sqlcmd.CommandText = @"select MerchandiseName from Merchandise";
            OleDbDataReader reader = sqlcmd.ExecuteReader();
            while(reader.Read())
            {
                listBox1.Items.Add(reader[0].ToString());
            }
            reader.Close();

            listBox1.SelectedIndex = 0;
            listBox1.Focus();
            SetText();
        }

        private void SetText()
        {
            textBox2.Text = "0";
            ExtingCountText.Text = "0";
        }

        private void AddMerchandise_Click(object sender, EventArgs e)
        {
            string strName = listBox1.SelectedItem.ToString();
            int stockCounts = System.Convert.ToInt32(textBox2.Text);
            int extingCounts = System.Convert.ToInt32(ExtingCountText.Text);
            OleDbConnection conn = MyDataBase.GetConn();

            if (AddMerchandise.Text.Equals("添加"))
            {
                string strCmd1 = @"update Merchandise set StockCounts=StockCounts+" + stockCounts.ToString();
                strCmd1 += @" where MerchandiseName='" + strName + "'";
                string strCmd2 = @"update Merchandise set ExitingCounts=ExitingCounts+" + stockCounts.ToString();
                strCmd2 += @" where MerchandiseName='" + strName + "'";

                int iRows = 0;
                sqlcmd.CommandText = strCmd1;
                iRows = sqlcmd.ExecuteNonQuery();
                if (iRows == 0)
                {
                    MessageBox.Show("添加失败，请查证后重新添加。");
                    return;
                }
                sqlcmd.CommandText = strCmd2;
                iRows = sqlcmd.ExecuteNonQuery();
                if (iRows == 0)
                {
                    MessageBox.Show("添加失败，请查证后重新添加。");
                    return;
                }
                SetText();

                //////////////////////////////////////////////////////////////////////////
                //添加出售信息
                string timeDate = System.DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff");
                string strCmd3 = @"INSERT INTO stockmerchandise ( merchandisename, timedate, stockcounts ) values('" + strName + "','" + timeDate + "'," + stockCounts.ToString() + ")";
                sqlcmd.CommandText = strCmd1;
                iRows = sqlcmd.ExecuteNonQuery();
                if (iRows == 0)
                {
                    MessageBox.Show("添加失败，请查证后重新添加。");
                    return;
                }
            }
            else if(AddMerchandise.Text.Equals("更新"))
            {
                string strCmd1 = @"update Merchandise set StockCounts=" + stockCounts.ToString();
                strCmd1 += @" where MerchandiseName='" + strName + "'";
                string strCmd2 = @"update Merchandise set ExitingCounts=" + extingCounts.ToString();
                strCmd2 += @" where MerchandiseName='" + strName + "'";

                int iRows = 0;
                sqlcmd.CommandText = strCmd1;
                iRows = sqlcmd.ExecuteNonQuery();
                if (iRows == 0)
                {
                    MessageBox.Show("更新失败，请查证后重新添加。");
                    return;
                }
                sqlcmd.CommandText = strCmd2;
                iRows = sqlcmd.ExecuteNonQuery();
                if (iRows == 0)
                {
                    MessageBox.Show("更新失败，请查证后重新添加。");
                    return;
                }
                ExtingCountText.Visible = false;
                ExtingCountLable.Visible = false;
                AddMerchandise.Text = "添加";
                SetText();
            }

            if (dataGridView1.SelectedRows.Count != 0)
            {
                if (dataGridView1.SelectedRows[0].Cells.Count > 1)
                {
                    DataGridViewRow dRow = dataGridView1.SelectedRows[0];
                    for (int i = 0; i < dataGridView1.RowCount; i++)
                    {
                        string str = dRow.Cells[0].Value.ToString();
                        Trace.WriteLine(str);
                        if (str.Equals(strName))
                        {
                            Trace.WriteLine("Enter if :");
                            dataGridView1.CurrentCell = dataGridView1[0, 1];
                        }
                    }
                }
            }

            UpdateDataGridView();
        }

        private void UpdateDataGridView()
        {
            OleDbConnection objConnection = MyDataBase.GetConn();

            sqlcmd.CommandText = @"select MerchandiseName,StockCounts,ExitingCounts from Merchandise";
            sqlcmd.Connection = objConnection;
            sqlcmd.CommandType = CommandType.Text;

            OleDbDataAdapter da = new OleDbDataAdapter();
            da.SelectCommand = sqlcmd;
            DataSet ds = new DataSet();
            da.Fill(ds, "table1");

            dataGridView1.DataSource = ds.Tables[0];

            dataGridView1.Columns[0].HeaderCell.Value = "商品名称";
            dataGridView1.Columns[1].HeaderCell.Value = "库存";
            dataGridView1.Columns[2].HeaderCell.Value = "现存";
        }

        private void Exit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

    }
}
