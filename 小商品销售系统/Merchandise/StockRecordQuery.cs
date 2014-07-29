using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Data.OleDb;
using System.Collections;
using System.Diagnostics;

namespace Merchandise
{
    public partial class StockRecordQuery : Form
    {
        OleDbCommand sqlCmd = new OleDbCommand();
        private CharacterRestrain characterRestrain = new CharacterRestrain();

        public StockRecordQuery()
        {
            InitializeComponent();
        }

        private void StockRecordQuery_Load(object sender, EventArgs e)
        {
            toolTip1.SetToolTip(timedateText, "日期格式:2011-01-01");
            toolTip1.Active = true;
            toolTip1.UseAnimation = true;
            timedateText.KeyPress += new KeyPressEventHandler(characterRestrain.TimedateRestrain_KeyPress);
            

            merchandiseNameCombo.Items.Add("");
            sqlCmd.CommandType = CommandType.Text;
            sqlCmd.Connection = MyDataBase.GetConn();
            sqlCmd.CommandText = @"select merchandisename from merchandise";
            OleDbDataReader reader = sqlCmd.ExecuteReader();

            while (reader.Read())
            {
                merchandiseNameCombo.Items.Add(reader[0].ToString());
            }
            reader.Close();

            merchandiseNameCombo.Text = merchandiseNameCombo.Items[0].ToString();
        }


        private string BuildSql()
        {
            ArrayList sqlList = new ArrayList();
            string str = "";
            if (!merchandiseNameCombo.Text.Trim().Equals(""))
            {
                str = " StockMerchandise.merchandisename='" + merchandiseNameCombo.Text.Trim() + "' ";
                sqlList.Add(str);
            }
            if (!timedateText.Text.Trim().Equals(""))
            {
                str = @" timedate like '%" + timedateText.Text.Trim() + "%' ";
                sqlList.Add(str);
            }
            string strCmd = @"SELECT StockMerchandise.merchandisename,timedate,StockMerchandise.stockcounts,merchandise.costprice from merchandise,StockMerchandise where StockMerchandise.merchandisename=merchandise.merchandisename ";

            if (sqlList.Count > 0)
            {
                str = " and" + sqlList[0].ToString();
                if (sqlList.Count == 2)
                {
                    str += " and " + sqlList[1].ToString();
                }
                strCmd += str;
            }

            return strCmd;
        }

        private void queryBtn_Click(object sender, EventArgs e)
        {
            string strCmd = BuildSql();
            Trace.WriteLine(strCmd);
            sqlCmd.CommandText = strCmd;
            OleDbDataAdapter da = new OleDbDataAdapter();
            da.SelectCommand = sqlCmd;
            DataSet ds = new DataSet();
            da.Fill(ds, "table1");

            dataGridView1.DataSource = ds.Tables[0];

            SetDataGridViewHeaderText();
        }

        private void SetDataGridViewHeaderText()
        {
            dataGridView1.Columns[0].HeaderText = "商品名称";
            dataGridView1.Columns[1].HeaderText = "时间日期";
            dataGridView1.Columns[2].HeaderText = "进货数量";
            dataGridView1.Columns[3].HeaderText = "成本价格";

            dataGridView1.Columns[1].Width = 160;
        }

        private void exportQueryBtn_Click(object sender, EventArgs e)
        {
            if (dataGridView1.Rows.Count == 0)
            {
                MessageBox.Show("数据表中没有要保存的数据");
                return;
            }
            string saveFileName = "";
            SaveFileDialog saveDialog = new SaveFileDialog();
            saveDialog.DefaultExt = "xls";
            saveDialog.Filter = "Excel文件|*.xls";
            saveDialog.FileName = "Sheet1";
            saveDialog.ShowDialog();
            saveFileName = saveDialog.FileName;
            if (saveFileName.IndexOf(":") < 0) return; //被点了取消 

            ExportExcel excel = new ExportExcel();
            excel.SaveExcel(saveFileName, dataGridView1);
        }
    }
}
