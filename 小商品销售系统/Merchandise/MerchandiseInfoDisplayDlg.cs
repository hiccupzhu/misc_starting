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
    public partial class MerchandiseInfoDisplayDlg : Form
    {
        OleDbCommand sqlCmd = new OleDbCommand();
        private CharacterRestrain characterRestrain = new CharacterRestrain();

        public MerchandiseInfoDisplayDlg()
        {
            InitializeComponent();
        }

        private void exportBtn_Click(object sender, EventArgs e)
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
            if (saveFileName.IndexOf(":") < 0)                return; //被点了取消 

            ExportExcel excel = new ExportExcel();
            excel.SaveExcel(saveFileName, dataGridView1);

        }

        private void MerchandiseInfoDisplayDlg_Load(object sender, EventArgs e)
        {
            stockCountsText.KeyPress +=new KeyPressEventHandler(characterRestrain.IntRestrain_KeyPress);
            existingCountsText.KeyPress +=new KeyPressEventHandler(characterRestrain.IntRestrain_KeyPress);
            comboBox1.Items.Add("");
            sqlCmd.CommandType = CommandType.Text;
            sqlCmd.Connection = MyDataBase.GetConn();
            sqlCmd.CommandText = @"select merchandisename from merchandise";
            OleDbDataReader reader = sqlCmd.ExecuteReader();

            while(reader.Read())
            {
                comboBox1.Items.Add(reader[0].ToString());
            }
            reader.Close();

            comboBox1.Text = comboBox1.Items[0].ToString();

            InitCombo();
        }

        private void InitCombo()
        {
            string[] condition = { "", ">", "=", "<", ">=", "<=" };
            for (int i = 0; i < 6; i++)
            {
                stockCondition.Items.Add(condition[i]);
                existingCondition.Items.Add(condition[i]);
            }
        }

        private void queryBtn_Click(object sender, EventArgs e)
        {
            string strCmd = BuildSql();
            Trace.WriteLine(strCmd);
            sqlCmd.CommandText = strCmd;
            OleDbDataAdapter da = new OleDbDataAdapter();
            da.SelectCommand = sqlCmd;
            DataSet ds = new DataSet();
            da.Fill(ds, "tableQuery");

            ds.Tables[0].Columns[0].ColumnName = "商品名称";
            ds.Tables[0].Columns[1].ColumnName = "库存";
            ds.Tables[0].Columns[2].ColumnName = "现存";
            ds.Tables[0].Columns[3].ColumnName = "成本价";
            dataGridView1.DataSource = ds.Tables[0];
        }

        private string BuildSql()
        {
            string strCmd = @"select * from merchandise";
            string str = "";
            ArrayList conditionList = new ArrayList();
            if(!comboBox1.Text.Trim().Equals(""))
            {
                str = @"merchandisename='" + comboBox1.Text.Trim() + "'";
                conditionList.Add(str);
            }
            if(!stockCondition.Text.Trim().Equals(""))
            {
                str = "StockCounts" + stockCondition.Text.Trim() + stockCountsText.Text.Trim();
                conditionList.Add(str);
            }

            if (!existingCondition.Text.Trim().Equals(""))
            {
                str = "ExitingCounts" + existingCondition.Text.Trim() + existingCountsText.Text.Trim();
                conditionList.Add(str);
            }

            if (conditionList.Count > 0)
            {
                str = conditionList[0].ToString();
                for (int i = 1; i < conditionList.Count; i++)
                {
                    str += " and " + conditionList[i];
                }

                strCmd += " where " + str;
            }
            
            return strCmd;
        }
    }
}
