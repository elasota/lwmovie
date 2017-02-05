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
    public partial class TheoraVideoOptionsControl : UserControl, ICodecSettingsControl
    {
        private List<KeyValuePair<string, int>> _bitratePresets = new List<KeyValuePair<string, int>>();

        public TheoraVideoOptionsControl()
        {
            InitializeComponent();

            _bitratePresets.Add(new KeyValuePair<string, int>("3.5 Mbps (Low DVD)", 3500));
            _bitratePresets.Add(new KeyValuePair<string, int>("5 Mbps (High DVD)", 5000));
            _bitratePresets.Add(new KeyValuePair<string, int>("12.5 Mbps", 12500));
            _bitratePresets.Add(new KeyValuePair<string, int>("25 Mbps (BRD Med)", 25000));
            _bitratePresets.Add(new KeyValuePair<string, int>("35 Mbps (BRD High)", 35000));

            foreach (KeyValuePair<string, int> preset in _bitratePresets)
                cbxBitratePresets.Items.Add(preset.Key);
        }

        private void cbxBitratePresets_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox)sender;
            if (cbx.SelectedIndex == 0)
            {
                txtBitrate.ReadOnly = false;
                txtBitrate.Visible = true;
                lblBitrateMaxSuffix.Visible = true;
            }
            else
            {
                txtBitrate.ReadOnly = true;
                txtBitrate.Visible = false;
                lblBitrateMaxSuffix.Visible = false;
                txtBitrate.Text = _bitratePresets[cbx.SelectedIndex - 1].Value.ToString();
            }
        }

        private void tbQuality_ValueChanged(object sender, EventArgs e)
        {
            lblQualityLevel.Text = ((TrackBar)sender).Value.ToString();
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            TheoraVideoSettings settings = new TheoraVideoSettings();
            settings.BitratePresetIndex = cbxBitratePresets.SelectedIndex;
            settings.Bitrate = int.Parse(txtBitrate.Text);
            settings.Use2Pass = chkTwoPass.Checked;
            settings.RateBufferSize = (int)nbBacklogSize.Value;
            settings.HaveKeyFrameRate = chkKeyFrames.Checked;
            settings.KeyFrameRate = (int)nbKeyRate.Value;
            settings.Quality = tbQuality.Value;
            settings.UseQuality = radUseQuality.Checked;
            settings.UseCBR = radUseCbr.Checked;
            settings.SubsampleChroma = !chkFullPrecisionChroma.Checked;
            settings.UseFullRangeYUV = chkFullRange.Checked;
            settings.SpeedLevel = cbxEncoderSpeed.SelectedIndex;

            return settings;
        }
        
        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
            TheoraVideoSettings settings = (TheoraVideoSettings)planSettings;
            cbxBitratePresets.SelectedIndex = settings.BitratePresetIndex;
            txtBitrate.Text = settings.Bitrate.ToString();

            chkTwoPass.Checked = settings.Use2Pass;
            nbBacklogSize.Value = settings.RateBufferSize;
            chkKeyFrames.Checked = settings.HaveKeyFrameRate;
            nbKeyRate.Value = settings.KeyFrameRate;
            tbQuality.Value = settings.Quality;
            radUseQuality.Checked = settings.UseQuality;
            radUseCbr.Checked = settings.UseCBR;
            chkFullPrecisionChroma.Checked = !settings.SubsampleChroma;
            chkFullRange.Checked = settings.UseFullRangeYUV;
            cbxEncoderSpeed.SelectedIndex = settings.SpeedLevel;
        }
    }
}
