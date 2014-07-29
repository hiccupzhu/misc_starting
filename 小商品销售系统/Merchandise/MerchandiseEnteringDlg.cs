using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;

using System.Text;
using System.Windows.Forms;
using System.Data.OleDb;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace Merchandise
{
    public partial class MerchandiseEnteringDlg : Form
    {
        private OleDbCommand sqlcmd = new OleDbCommand();
        private CharacterRestrain characterRestrain = new CharacterRestrain();

        public MerchandiseEnteringDlg()
        {
            InitializeComponent();
            this.Load += new EventHandler(MerchandiseEnteringDlg_Load);
            dataGridView1.MouseClick += new MouseEventHandler(dataGridView1_MouseClick);
            dataGridView1.LostFocus +=new EventHandler(dataGridView1_LostFocus);
            stockCountsText.KeyPress += new KeyPressEventHandler(characterRestrain.IntRestrain_KeyPress);
            extingCountsText.KeyPress += new KeyPressEventHandler(characterRestrain.IntRestrain_KeyPress);
            characterRestrain.textBox = costPriceText;
            costPriceText.KeyPress += new KeyPressEventHandler(characterRestrain.DoubleRestrain_KeyPress);
        }
        

        private void MerchandiseEnteringDlg_Load(Object sender,EventArgs e)
        {
            UpdateDataGridView();
            ClearText();

            DeleteBtn.Enabled = false;
        }

        private void dataGridView1_MouseClick(Object sender,MouseEventArgs e)
        {
            DeleteBtn.Enabled = true;
        }

        private void dataGridView1_LostFocus(Object sender,EventArgs e)
        {
//            DeleteBtn.Enabled = false;
        }

        private void UpdateDataGridView()
        {
            OleDbConnection objConnection = MyDataBase.GetConn();

            sqlcmd.CommandText = @"select * from Merchandise";
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
            dataGridView1.Columns[3].HeaderCell.Value = "成本价";
        }

        private void ExitBtn_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void EnteringBtn_Click(object sender, EventArgs e)
        {
            string strName=MerchandiseName.Text.Trim();
            int stockCounts = System.Convert.ToInt32(stockCountsText.Text.Trim());
            int existingCounts = System.Convert.ToInt32(extingCountsText.Text.Trim());
            double costPrice = System.Convert.ToDouble(costPriceText.Text.Trim());

            sqlcmd.CommandText = @"select MerchandiseName from Merchandise";
            OleDbDataReader reader = sqlcmd.ExecuteReader();
            while (reader.Read())
            {
                if(reader[0].ToString().Equals(strName))
                {
                    MessageBox.Show("存在相同的商品名称，请查证之后重新操作。");
                    return;                    
                }
            }
            reader.Close();

            string strCmd = @"INSERT INTO Merchandise(MerchandiseName,stockcounts,exitingcounts,costprice) VALUES('";
            strCmd += strName + @"',";
            strCmd += stockCounts.ToString() + ",";
            strCmd += existingCounts.ToString() + ",";
            strCmd += costPrice.ToString() + ")";
            sqlcmd.CommandText = strCmd;
            int iRows = 0;
            iRows = sqlcmd.ExecuteNonQuery();
            if(0==iRows)
            {
                MessageBox.Show("添加记录失败，请查证后重试。");
                return;
            }

            ClearText();
            UpdateDataGridView();
        }

        private void DeleteBtn_Click(object sender, EventArgs e)
        {
            int selectedRow = dataGridView1.SelectedRows[0].Index;
            string strName=dataGridView1.SelectedRows[0].Cells[0].Value.ToString();

            string strCmd = @"delete from Merchandise where MerchandiseName='" + strName + "'";
            sqlcmd.CommandText = strCmd;
            int iRows = sqlcmd.ExecuteNonQuery();
            if(0==iRows)
            {
                MessageBox.Show("删除记录失败。");
                return;
            }

            UpdateDataGridView();
            DeleteBtn.Enabled = false;
            MessageBox.Show("记录删除成功。");
        }

        private void ClearText()
        {
            MerchandiseName.Text = "";
            stockCountsText.Text = "0";
            extingCountsText.Text = "0";
            costPriceText.Text = "0.0";
        }

        private void MerchandiseEnteringDlg_Load_1(object sender, EventArgs e)
        {

        }
    }
}
