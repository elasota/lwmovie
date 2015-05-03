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
    public partial class ROQVideoOptionsControl : UserControl, ICodecSettingsControl
    {
        private List<KeyValuePair<string, int>> _bitratePresets = new List<KeyValuePair<string, int>>();

        public ROQVideoOptionsControl()
        {
            InitializeComponent();
            
            _bitratePresets.Add(new KeyValuePair<string, int>("500 Kbps", 500000));
            _bitratePresets.Add(new KeyValuePair<string, int>("1 Mbps", 1000000));
            _bitratePresets.Add(new KeyValuePair<string, int>("2 Mbps", 2000000));
            _bitratePresets.Add(new KeyValuePair<string, int>("4 Mbps (Low DVD)", 4000000));
            _bitratePresets.Add(new KeyValuePair<string,int>("6 Mbps (High DVD)", 6000000));
            _bitratePresets.Add(new KeyValuePair<string,int>("15 Mbps", 15000000));

            foreach (KeyValuePair<string, int> preset in _bitratePresets)
                cbxBitratePreset.Items.Add(preset.Key);
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            ROQVideoSettings settings = new ROQVideoSettings();

            settings.BitratePresetIndex = cbxBitratePreset.SelectedIndex;
            int temp;
            if (int.TryParse(txtBitrate.Text, out temp))
                settings.Bitrate = temp;
            if (int.TryParse(txtKeyFrameRate.Text, out temp))
                settings.KeyFrameRate = temp;
            settings.NumCBPhases = tbNumCBPhases.Value;
            settings.ThresholdPower = tbCBThreshold.Value;

            return settings;
        }

        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
            ROQVideoSettings settings = (ROQVideoSettings)planSettings;
            cbxBitratePreset.SelectedIndex = settings.BitratePresetIndex;
            if (settings.BitratePresetIndex == 0)
                txtBitrate.Text = settings.Bitrate.ToString();
            txtKeyFrameRate.Text = settings.KeyFrameRate.ToString();
            tbNumCBPhases.Value = settings.NumCBPhases;
            tbCBThreshold.Value = settings.ThresholdPower;
        }

        private void cbxBitratePreset_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox)sender;
            if (cbx.SelectedIndex == 0)
            {
                txtBitrate.Visible = true;
                lblBitrateSuffix.Visible = true;
            }
            else
            {
                txtBitrate.Text = _bitratePresets[cbx.SelectedIndex - 1].Value.ToString();
                txtBitrate.Visible = false;
                lblBitrateSuffix.Visible = false;
            }
        }
    }
}
