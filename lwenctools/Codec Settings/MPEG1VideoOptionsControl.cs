using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace lwenctools
{
    public partial class MPEG1VideoOptionsControl : UserControl, ICodecSettingsControl
    {
        private List<KeyValuePair<string, int>> _maxBitratePresets = new List<KeyValuePair<string, int>>();
        private List<KeyValuePair<string, int>> _minBitratePresets = new List<KeyValuePair<string, int>>();
        private List<KeyValuePair<string, int>> _motionEstimationPresets = new List<KeyValuePair<string, int>>();

        public MPEG1VideoOptionsControl()
        {
            InitializeComponent();

            _maxBitratePresets.Add(new KeyValuePair<string, int>("4 Mbps (Low DVD)", 4000));
            _maxBitratePresets.Add(new KeyValuePair<string,int>("6 Mbps (High DVD)", 6000));
            _maxBitratePresets.Add(new KeyValuePair<string,int>("15 Mbps", 15000));
            _maxBitratePresets.Add(new KeyValuePair<string,int>("30 Mbps (BRD Med)", 30000));
            _maxBitratePresets.Add(new KeyValuePair<string,int>("40 Mbps (BRD High)", 40000));

            _minBitratePresets.Add(new KeyValuePair<string, int>("3 Mbps (Low DVD)", 3000));
            _minBitratePresets.Add(new KeyValuePair<string, int>("4 Mbps (High DVD)", 4000));
            _minBitratePresets.Add(new KeyValuePair<string, int>("10 Mbps", 10000));
            _minBitratePresets.Add(new KeyValuePair<string, int>("20 Mbps (BRD Med)", 20000));
            _minBitratePresets.Add(new KeyValuePair<string, int>("30 Mbps (BRD High)", 30000));
            
            _motionEstimationPresets.Add(new KeyValuePair<string, int>("Default", 0));
            _motionEstimationPresets.Add(new KeyValuePair<string, int>("256", 256));
            _motionEstimationPresets.Add(new KeyValuePair<string, int>("512", 512));
            _motionEstimationPresets.Add(new KeyValuePair<string, int>("768", 768));
            _motionEstimationPresets.Add(new KeyValuePair<string, int>("1024 (Best)", 1024));

            foreach (KeyValuePair<string, int> preset in _maxBitratePresets)
                cbxBitrateMaxPresets.Items.Add(preset.Key);
            foreach (KeyValuePair<string, int> preset in _minBitratePresets)
                cbxBitrateMinPresets.Items.Add(preset.Key);
            foreach (KeyValuePair<string, int> preset in _motionEstimationPresets)
                cbxMotionEstimation.Items.Add(preset.Key);
        }

        private void tbQuality_ValueChanged(object sender, EventArgs e)
        {
            lblQualityLevel.Text = ((TrackBar)sender).Value.ToString();
        }

        private void cbxBitrateMaxPresets_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox)sender;
            if (cbx.SelectedIndex == 0)
            {
                txtBitrateMax.ReadOnly = false;
                txtBitrateMax.Visible = true;
                lblBitrateMaxSuffix.Visible = true;
            }
            else
            {
                txtBitrateMax.ReadOnly = true;
                txtBitrateMax.Visible = false;
                lblBitrateMaxSuffix.Visible = false;
                txtBitrateMax.Text = _maxBitratePresets[cbx.SelectedIndex - 1].Value.ToString();
            }
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            MPEG1VideoSettings settings = new MPEG1VideoSettings();
            settings.MaxSlices = int.Parse(txtSlices.Text);
            settings.BitrateBufferSize = int.Parse(txtBitrateBufferSize.Text);
            settings.BitrateMinPresetIndex = cbxBitrateMinPresets.SelectedIndex;
            settings.BitrateMaxPresetIndex = cbxBitrateMaxPresets.SelectedIndex;
            if (settings.BitrateMaxPresetIndex == 0)
                settings.BitrateMax = int.Parse(txtBitrateMax.Text);
            else
                settings.BitrateMax = _maxBitratePresets[settings.BitrateMaxPresetIndex - 1].Value;
            if (settings.BitrateMinPresetIndex == 0)
                settings.BitrateMin = int.Parse(txtBitrateMin.Text);
            else
                settings.BitrateMin = _minBitratePresets[settings.BitrateMinPresetIndex - 1].Value;
            settings.NumBFrames = cbxBFrameGroupSize.SelectedIndex;
            settings.BStrategy = cbxBStrategy.SelectedIndex;
            settings.Quality = tbQuality.Value;
            settings.AutoRD = chkAutoRD.Checked;
            settings.FullRange = chkFullRange.Checked;
            settings.TwoPass = chkTwoPass.Checked;
            settings.MotionEstimationPresetIndex = cbxMotionEstimation.SelectedIndex;
            settings.UseQuality = radUseQuality.Checked;
            settings.UseCBR = radUseCbr.Checked;

            return settings;
        }

        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
            MPEG1VideoSettings settings = (MPEG1VideoSettings)planSettings;
            this.txtSlices.Text = settings.MaxSlices.ToString();
            this.txtBitrateBufferSize.Text = settings.BitrateBufferSize.ToString();
            this.cbxBitrateMaxPresets.SelectedIndex = settings.BitrateMaxPresetIndex;
            this.cbxBitrateMinPresets.SelectedIndex = settings.BitrateMinPresetIndex;
            if (settings.BitrateMaxPresetIndex == 0)
                this.txtBitrateMax.Text = settings.BitrateMax.ToString();
            if (settings.BitrateMinPresetIndex == 0)
                this.txtBitrateMin.Text = settings.BitrateMin.ToString();
            this.cbxBFrameGroupSize.SelectedIndex = settings.NumBFrames;
            this.cbxBStrategy.SelectedIndex = settings.BStrategy;
            this.tbQuality.Value = settings.Quality;
            this.chkAutoRD.Checked = settings.AutoRD;
            this.chkFullRange.Checked = settings.FullRange;
            this.chkTwoPass.Checked = settings.TwoPass;
            this.cbxMotionEstimation.SelectedIndex = settings.MotionEstimationPresetIndex;
            this.radUseQuality.Checked = settings.UseQuality;
            this.radUseCbr.Checked = settings.UseCBR;
        }

        private void cbxBitrateMinPresets_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox)sender;
            if (cbx.SelectedIndex == 0)
            {
                txtBitrateMin.Visible = true;
                lblBitrateMinSuffix.Visible = true;
            }
            else
            {
                txtBitrateMin.Text = _minBitratePresets[cbx.SelectedIndex - 1].Value.ToString();
                txtBitrateMin.Visible = false;
                lblBitrateMinSuffix.Visible = false;
            }
        }
    }
}
