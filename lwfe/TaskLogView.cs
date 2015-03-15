using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwfe
{
    public partial class TaskLogView : UserControl
    {
        public TaskLogView()
        {
            InitializeComponent();
        }

        public string ProcessPath
        {
            get
            {
                return txtProcessPath.Text;
            }
            set
            {
                txtProcessPath.Text = value;
            }
        }

        public string Arguments
        {
            get
            {
                return txtArguments.Text;
            }
            set
            {
                txtArguments.Text = value;
            }
        }

        public void AppendText(string text)
        {
            txtConsoleOutput.AppendText(text);
        }
    }
}
