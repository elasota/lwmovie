using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwenctools
{
    public partial class ADPCMAudioOptionsControl : UserControl, ICodecSettingsControl
    {
        public ADPCMAudioOptionsControl()
        {
            InitializeComponent();
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            ADPCMAudioSettings settings = new ADPCMAudioSettings();
            return settings;
        }

        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
        }

        private void ADPCMAudioOptionsControl_Load(object sender, EventArgs e)
        {
        
        }
    }
}
