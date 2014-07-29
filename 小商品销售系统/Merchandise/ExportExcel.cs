using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data;
using Microsoft.Office.Interop.Excel;
using Microsoft.Office.Interop;
using System.Diagnostics;

namespace Merchandise
{
    public class ExportExcel
    {
        public void SaveExcel(string saveFileName, DataGridView dataGridView1)
        {
            if (dataGridView1.Rows.Count > 0)
            {
                Microsoft.Office.Interop.Excel.Application app;
                Workbooks workBooks;
                Workbook workBook;
                Worksheet workSheet;

                try
                {
                    app = new Microsoft.Office.Interop.Excel.Application();
                    workBooks = app.Workbooks;
                    workBook = workBooks.Add(XlWBATemplate.xlWBATWorksheet);
                    workSheet = (Worksheet)workBook.Worksheets[1];
                }
                catch (System.Exception ex)
                {
                    Trace.WriteLine(ex.ToString());
                    MessageBox.Show(" 您可能没有安装 Office ，请安装再使用该功能 ");                    
                    return;
                }
                

                for (int i = 0; i < dataGridView1.Columns.Count; i++)
                {
                    workSheet.Cells[1, i + 1] = dataGridView1.Columns[i].HeaderText;
                }

                for (int i = 0; i < dataGridView1.Rows.Count - 1; i++)
                {
                    for (int j = 0; j < dataGridView1.Columns.Count; j++)
                    {
                        if (dataGridView1[j, i].ValueType == typeof(string))
                        {
                            workSheet.Cells[i + 2, j + 1] = "'" + dataGridView1[j, i].Value.ToString();
                        }
                        else
                        {
                            workSheet.Cells[i + 2, j + 1] = dataGridView1[j, i].Value.ToString();
                        }
                    }
                }


                if (saveFileName != "")
                {
                    try
                    {
                        workBook.Saved = true;
                        workBook.SaveCopyAs(saveFileName);

                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("导出文件时出错,文件可能正被打开！\n" + ex.Message);
                    }

                }

                app.Quit();
                app = null;
            }
            else
            {
                MessageBox.Show("数据表中没有要保存的数据");
            }
        }
        
    
    }
}
