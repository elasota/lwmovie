using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwfe
{
    public partial class ErrorList : Form
    {
        public ErrorList()
        {
            InitializeComponent();
        }

        public void AddError(string error)
        {
            txtErrors.AppendText(error + Environment.NewLine);
        }

        private void ErrorList_Load(object sender, EventArgs e)
        {
            this.CenterToScreen();
        }
    }
}
