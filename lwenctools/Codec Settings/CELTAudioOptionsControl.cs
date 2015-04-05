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
    public partial class CELTAudioOptionsControl : UserControl, ICodecSettingsControl
    {
        private List<KeyValuePair<string, int>> _bitratePresets = new List<KeyValuePair<string, int>>();

        public CELTAudioOptionsControl()
        {
            InitializeComponent();

            _bitratePresets.Add(new KeyValuePair<string, int>("64 kbps", 64000));
            _bitratePresets.Add(new KeyValuePair<string, int>("72 kbps", 72000));
            _bitratePresets.Add(new KeyValuePair<string, int>("96 kbps", 96000));
            _bitratePresets.Add(new KeyValuePair<string, int>("128 kbps", 128000));
            _bitratePresets.Add(new KeyValuePair<string, int>("160 kbps", 160000));
            _bitratePresets.Add(new KeyValuePair<string, int>("192 kbps", 192000));
            _bitratePresets.Add(new KeyValuePair<string, int>("256 kbps", 256000));
            _bitratePresets.Add(new KeyValuePair<string, int>("320 kbps", 320000));

            foreach (KeyValuePair<string, int> kvp in _bitratePresets)
                cbxBitratePreset.Items.Add(kvp.Key);
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            CELTAudioSettings settings = new CELTAudioSettings();
            settings.BitRatePresetIndex = cbxBitratePreset.SelectedIndex;
            settings.BitRate = int.Parse(txtBitRate.Text);
            settings.VBR = chkVBR.Checked;

            return settings;
        }

        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
            CELTAudioSettings settings = (CELTAudioSettings)planSettings;
            cbxBitratePreset.SelectedIndex = settings.BitRatePresetIndex;
            txtBitRate.Text = settings.BitRate.ToString();
            chkVBR.Checked = settings.VBR;
        }

        private void cbxBitratePreset_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox)sender;
            if (cbx.SelectedIndex == 0)
            {
                txtBitRate.Visible = true;
                lblBPS.Visible = true;
            }
            else
            {
                txtBitRate.Visible = false;
                lblBPS.Visible = false;
                txtBitRate.Text = _bitratePresets[cbx.SelectedIndex - 1].Value.ToString();
            }
        }
    }
}
