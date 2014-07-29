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
    public partial class SaleRecordQueryDlg : Form
    {
        OleDbCommand sqlCmd = new OleDbCommand();
        private CharacterRestrain characterRestrain = new CharacterRestrain();

        public SaleRecordQueryDlg()
        {
            InitializeComponent();
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
            DataTable dt = ds.Tables[0];

            double costSumPrice = 0.0;
            double saleSumPrice = 0.0;
            int saleSumCounts = 0;
            double income = 0.0;
            for(int i=0;i<dt.Rows.Count;i++)
            {
                double costPrice = System.Convert.ToDouble(dt.Rows[i][2].ToString());
                double salePrice = System.Convert.ToDouble(dt.Rows[i][3].ToString());
                int saleCounts = System.Convert.ToInt32(dt.Rows[i][4].ToString());
                costSumPrice += costPrice;
                saleSumPrice += salePrice;
                saleSumCounts += saleCounts;
                income = (salePrice - costPrice) * saleCounts;
            }

            DataRow dr1=dt.NewRow();
            dr1[0] = "合计";
            dr1[1]= System.DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss:ffff");
            dr1[2] = costSumPrice.ToString();
            dr1[3] = saleSumPrice.ToString();
            dr1[4] = saleSumCounts.ToString();
            dt.Rows.Add(dr1);

            DataRow dr2 = dt.NewRow();
            dr2[0] = "总收入";
            dr2[3] = income.ToString();            
            dt.Rows.Add(dr2);

            dataGridView1.DataSource = ds.Tables[0];

            SetDataGridViewHeaderText();
        }

        private void SetDataGridViewHeaderText()
        {
            dataGridView1.Columns[0].HeaderText = "商品名称";
            dataGridView1.Columns[1].HeaderText = "时间日期";
            dataGridView1.Columns[2].HeaderText = "成本价";
            dataGridView1.Columns[3].HeaderText = "出售价";
            dataGridView1.Columns[4].HeaderText = "出售数量";

            dataGridView1.Columns[1].Width = 160;
        }

        private void SaleRecordQueryDlg_Load(object sender, EventArgs e)
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
            if(!merchandiseNameCombo.Text.Trim().Equals(""))
            {
                str = " salerecord.merchandisename='" + merchandiseNameCombo.Text.Trim() + "' ";
                sqlList.Add(str);
            }
            if(!timedateText.Text.Trim().Equals(""))
            {
                str = @" timedate like '%" + timedateText.Text.Trim() + "%' ";
                sqlList.Add(str);
            }
            string strCmd = @"SELECT SaleRecord.merchandisename,timedate,merchandise.costprice,saleprice,salecounts from merchandise,salerecord where salerecord.merchandisename=merchandise.merchandisename ";

            if (sqlList.Count > 0)
            {
                str = " and" + sqlList[0].ToString();
                if(sqlList.Count==2)
                {
                    str += " and " + sqlList[1].ToString();
                }
                strCmd += str;
            }

            return strCmd;
        }

        private void exportExcelBtn_Click(object sender, EventArgs e)
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
