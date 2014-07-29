using System;
using System.Collections.Generic;

using System.Text;
using System.Data.OleDb;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Windows.Forms;


namespace Merchandise
{
    class MyDataBase
    {
        private static string strConnection = @"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=";
        private static OleDbConnection objConnection = new OleDbConnection();
        private static bool isOpened = false;

        public static OleDbConnection GetConn()
        {            
            if (!isOpened)
            {
                strConnection += System.IO.Directory.GetCurrentDirectory() + @"\database\db1.mdb";
                objConnection.ConnectionString = strConnection;
                objConnection.Open();
                isOpened = true;
            }
            return  objConnection;
        }

        public static void CloseConn()
        {
            if (isOpened)
            {
                objConnection.Close();
                isOpened = false;
            }
        }        
    }

    class CharacterRestrain
    {
        public void IntRestrain_KeyPress(Object sender, KeyPressEventArgs e)
        {
            if (!(e.KeyChar >= 48 && e.KeyChar <= 57 || e.KeyChar == 8))
            {
                e.Handled = true;
            }
        }

        public TextBox textBox=null;
        public void DoubleRestrain_KeyPress(Object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar != 8)
            {
                Regex regex = new Regex(@"^[0-9]+[.]?[0-9]*$");
                if (!(regex.IsMatch(textBox.Text.Trim() + e.KeyChar.ToString())))
                {
                    Trace.WriteLine(textBox.Text.Trim());
                    e.Handled = true;
                }
            }
        }

        public void TimedateRestrain_KeyPress(Object sender, KeyPressEventArgs e)
        {
            if (!(e.KeyChar >= 48 && e.KeyChar <= 57 || e.KeyChar == 8 || e.KeyChar == '-'))
            {
                e.Handled = true;
            }
        }
    }
}
