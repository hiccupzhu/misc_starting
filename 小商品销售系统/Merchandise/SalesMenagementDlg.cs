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
    public partial class SalesMenagementDlg : Form
    {
        OleDbCommand sqlCmd = new OleDbCommand();
        private CharacterRestrain charactorRestrain = new CharacterRestrain();
        
        public SalesMenagementDlg()
        {
            InitializeComponent();
            listView1.MouseClick +=new MouseEventHandler(listView1_MouseClick);
        }
        private void listView1_MouseClick(Object sender,MouseEventArgs e)
        {
            deleteBtn.Enabled = true;
        }

        private void SalesMenagementDlg_Load(object sender, EventArgs e)
        {
            SetListViewFormat();
            FillComboBox1();
            ClearText();
            deleteBtn.Enabled=false;

            saleCountsText.KeyPress +=new KeyPressEventHandler(charactorRestrain.IntRestrain_KeyPress);
            charactorRestrain.textBox = salePriceText;
            salePriceText.KeyPress +=new KeyPressEventHandler(charactorRestrain.DoubleRestrain_KeyPress);
        }

        private void deleteBtn_Click(object sender, EventArgs e)
        {
            ListViewItem lvItem=listView1.SelectedItems[0];
            string timeDate=lvItem.SubItems[0].Text;
            string strName = lvItem.SubItems[1].Text;

            string strCmd = @"delete from salerecord where timedate='" + timeDate + "' and merchandisename='" + strName + "'";
            sqlCmd.CommandText = strCmd;
            int iRows = sqlCmd.ExecuteNonQuery();
            if(0==iRows)
            {
                MessageBox.Show("删除失败");
                return;
            }
            lvItem.Remove();

            deleteBtn.Enabled = false;
        }

        private void okBtn_Click(object sender, EventArgs e)
        {
            string strName = comboBox1.Text;
            int saleCounts = System.Convert.ToInt32(saleCountsText.Text.Trim());
            double salePrice = System.Convert.ToDouble(salePriceText.Text.Trim());
            string timeDate = System.DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff");
            Trace.WriteLine(timeDate);

            string strCmd = @"INSERT INTO SaleRecord(merchandisename,timedate,salecounts,saleprice) VALUES('";
            strCmd += strName + "','";
            strCmd += timeDate + "',";
            strCmd += saleCounts.ToString() + ",";
            strCmd += salePrice.ToString() + ")";
            sqlCmd.CommandText = strCmd;
            int iRows = sqlCmd.ExecuteNonQuery();
            if (0 == iRows)
            {
                MessageBox.Show("操作失败，请查证后重试。");
                return;
            }
            ListViewItem lvItem = new ListViewItem(timeDate);
            lvItem.SubItems.Add(strName);
            lvItem.SubItems.Add(saleCounts.ToString());
            lvItem.SubItems.Add(salePrice.ToString());
            listView1.Items.Add(lvItem);

            strCmd = @"update merchandise set exitingcounts=exitingcounts - " + saleCounts.ToString() + " where merchandisename='" + strName + "'";
            sqlCmd.CommandText = strCmd;
            Trace.WriteLine(strCmd);
            iRows = sqlCmd.ExecuteNonQuery();
            if (0 == iRows)
            {
                MessageBox.Show("操作失败，请查证后重试。");
                return;
            }
        }

        private void exitBtn_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void SetListViewFormat()
        {
            listView1.Columns.Add("时间日期");
            listView1.Columns.Add("商品名称");
            listView1.Columns.Add("卖出件数");
            listView1.Columns.Add("卖出价格");

            listView1.Columns[0].Width = 200;
            listView1.Columns[1].Width = 160;
            listView1.Columns[2].Width = 80;
            listView1.Columns[3].Width = 80;
        }

        private void FillComboBox1()
        {
            sqlCmd.Connection = MyDataBase.GetConn();
            sqlCmd.CommandType = CommandType.Text;
            sqlCmd.CommandText = "select MerchandiseName from Merchandise";
            OleDbDataReader reader = sqlCmd.ExecuteReader();

            while(reader.Read())
            {
                Trace.WriteLine(reader[0].ToString());
                comboBox1.Items.Add(reader[0].ToString());
            }
            reader.Close();

            comboBox1.Text = comboBox1.Items[0].ToString();
        }

        private void ClearText()
        {
            saleCountsText.Text = "0";
            salePriceText.Text = "0.0";
        }

        private void displayAllBtn_Click(object sender, EventArgs e)
        {
            listView1.Clear();
            SetListViewFormat();

            string strCmd = @"select * from salerecord";
            sqlCmd.CommandText = strCmd;
            OleDbDataReader reader = sqlCmd.ExecuteReader();

            while (reader.Read())
            {
                ListViewItem lvItem = new ListViewItem(reader[1].ToString());
                lvItem.SubItems.Add(reader[0].ToString());
                lvItem.SubItems.Add(reader[2].ToString());
                lvItem.SubItems.Add(reader[3].ToString());
                listView1.Items.Add(lvItem);
            }
        }
    
    }
}
