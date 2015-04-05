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
    public partial class MP2AudioOptionsControl : UserControl, ICodecSettingsControl
    {
        private List<KeyValuePair<string, int>> _bitratePresets = new List<KeyValuePair<string, int>>();

        public MP2AudioOptionsControl()
        {
            InitializeComponent();

            _bitratePresets.Add(new KeyValuePair<string, int>("32 kbps", 32000));
            _bitratePresets.Add(new KeyValuePair<string, int>("48 kbps", 48000));
            _bitratePresets.Add(new KeyValuePair<string, int>("56 kbps", 56000));
            _bitratePresets.Add(new KeyValuePair<string, int>("64 kbps", 64000));
            _bitratePresets.Add(new KeyValuePair<string, int>("80 kbps", 80000));
            _bitratePresets.Add(new KeyValuePair<string, int>("96 kbps", 96000));
            _bitratePresets.Add(new KeyValuePair<string, int>("112 kbps", 112000));
            _bitratePresets.Add(new KeyValuePair<string, int>("128 kbps", 128000));
            _bitratePresets.Add(new KeyValuePair<string, int>("160 kbps", 160000));
            _bitratePresets.Add(new KeyValuePair<string, int>("192 kbps", 192000));
            _bitratePresets.Add(new KeyValuePair<string, int>("224 kbps", 224000));
            _bitratePresets.Add(new KeyValuePair<string, int>("256 kbps", 256000));
            _bitratePresets.Add(new KeyValuePair<string, int>("320 kbps", 320000));
            _bitratePresets.Add(new KeyValuePair<string, int>("384 kbps", 384000));

            foreach (KeyValuePair<string, int> kvp in _bitratePresets)
                cbxBitratePreset.Items.Add(kvp.Key);
        }

        IExecutionPlanSettings ICodecSettingsControl.GenerateSettings()
        {
            MP2AudioSettings settings = new MP2AudioSettings();
            settings.BitRatePresetIndex = cbxBitratePreset.SelectedIndex;
            settings.BitRate = _bitratePresets[cbxBitratePreset.SelectedIndex].Value;

            return settings;
        }

        void ICodecSettingsControl.LoadFromSettings(IExecutionPlanSettings planSettings)
        {
            MP2AudioSettings settings = (MP2AudioSettings)planSettings;
            cbxBitratePreset.SelectedIndex = settings.BitRatePresetIndex;
        }
    }
}
