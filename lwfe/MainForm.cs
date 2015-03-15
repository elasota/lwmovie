using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace lwfe
{
    public partial class MainForm : Form
    {
        private Dictionary<Panel, PanelContainedControl> _panelContainedControls = new Dictionary<Panel,PanelContainedControl>();
        private EncodeSettingsProject _project = null;
        private ICodecSettingsControl _videoCodecSettingsControl = null;
        private ICodecSettingsControl _audioCodecSettingsControl = null;

        class PanelContainedControl
        {
            public Control Control { get; private set; }
            public Size ControlOriginalSize { get; private set; }

            public PanelContainedControl(Control c)
            {
                this.ControlOriginalSize = c.Size;
                this.Control = c;
            }
        };

        public MainForm()
        {
            InitializeComponent();
        }

        private void fsVideoEncodeInput_FileChanged(object sender, EventArgs e)
        {
            FileSelector fs = (FileSelector)sender;
            string strippedFName = fs.FileName;
            int dotIndex = strippedFName.LastIndexOf('.');
            if (dotIndex != -1)
                strippedFName = strippedFName.Substring(0, dotIndex);

            if (fsVideoEncodeOutput.FileName == "")
            {
                fsVideoEncodeOutput.FileName = strippedFName + ".lwiv";
                fsExistingVideoIntermediate.FileName = strippedFName + ".lwiv";
            }

            if (fsOutputFile.FileName == "")
                fsOutputFile.FileName = strippedFName + ".lwmv";

            if (_project != null)
                radVideoEncode.Checked = true;
        }

        private void SetPanelContainedControl(Panel p, Control c)
        {
            if (this._panelContainedControls.ContainsKey(p))
            {
                PanelContainedControl pcc = _panelContainedControls[p];
                p.Controls.Remove(pcc.Control);
                this._panelContainedControls.Remove(p);
            }
            if(c != null)
                _panelContainedControls[p] = new PanelContainedControl(c);
            RecalculateMinimumSize();
            if(c != null)
            {
                p.Controls.Add(c);
                c.Size = p.Size;
            }
        }

        private void RecalculateMinimumSize()
        {
            List<Panel> panels = new List<Panel>();
            panels.Add(pnlVideoCodecOptions);

            int minWidth = 0;
            int minHeight = 0;
            foreach (Panel p in panels)
            {
                int paddingWidth = this.Width - p.Width;
                int paddingHeight = this.Height - p.Height;
                if(_panelContainedControls.ContainsKey(p))
                {
                    Size ctrlOriginalSize = _panelContainedControls[p].ControlOriginalSize;

                    int totalWidth = ctrlOriginalSize.Width + paddingWidth;
                    int totalHeight = ctrlOriginalSize.Height + paddingHeight;
                    if (totalWidth > minWidth)
                        minWidth = totalWidth;
                    if (totalHeight > minHeight)
                        minHeight = totalHeight;
                }
            }
            this.MinimumSize = new Size(minWidth, minHeight);
        }

        private void cbxVideoCodec_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Save the current codec settings
            SaveCurrentVideoCodecSettings();

            ComboBox cbx = (ComboBox)sender;
            Control codecOptionsControl = null;
            if (cbx.SelectedIndex == 0)
                codecOptionsControl = CodecRepository.VideoCodecs[cbx.SelectedIndex].CreateCodecControl();

            SetPanelContainedControl(pnlVideoCodecOptions, codecOptionsControl);

            _videoCodecSettingsControl = (ICodecSettingsControl)codecOptionsControl;
        }

        private void pnlCodecOptions_Paint(object sender, PaintEventArgs e)
        {
            Panel p = (Panel)sender;
            foreach (Control c in p.Controls)
                c.Size = p.Size;
        }

        private void LoadProject(EncodeSettingsProject proj)
        {
            this._project = null;

            IExecutionPlanSettings selectedCodecSettings;

            for (int i = 0; i < CodecRepository.VideoCodecs.Length; i++)
            {
                if (CodecRepository.VideoCodecs[i].CodecID == proj.VideoCodecID)
                {
                    cbxVideoCodec.SelectedIndex = i;
                    break;
                }
            }

            LoadVideoCodecSettings(proj);

            for (int i = 0; i < CodecRepository.AudioCodecs.Length; i++)
            {
                if (CodecRepository.AudioCodecs[i].CodecID == proj.AudioCodecID)
                {
                    cbxAudioCodec.SelectedIndex = i;
                    break;
                }
            }

            LoadAudioCodecSettings(proj);

            fsExistingVideoIntermediate.FileName = proj.VideoIntermediateFile;
            fsVideoEncodeInput.FileName = proj.VideoEncodeInputFile;
            fsVideoEncodeOutput.FileName = proj.VideoEncodeOutputFile;
            radVideoUseIF.Checked = proj.VideoUseIntermediate;
            radVideoEncode.Checked = !proj.VideoUseIntermediate;

            this._project = proj;
        }

        private void SaveCurrentVideoCodecSettings()
        {
            if (_project != null)
                _project.VideoCodecSettings[_project.VideoCodecID] = _videoCodecSettingsControl.GenerateSettings();
        }

        private void SaveCurrentAudioCodecSettings()
        {
            if (_project != null)
            {
                if (_project.SelectedAudioStreamIndex != -1)
                {
                    EncodeSettingsProject.AudioStreamSettings audioStreamSettings = _project.AudioStreams[_project.SelectedAudioStreamIndex];
                    audioStreamSettings.AudioCodecSettings[_project.AudioCodecID] = _audioCodecSettingsControl.GenerateSettings();
                    audioStreamSettings.EncodeInputFile = fsAudioEncodeInput.FileName;
                    audioStreamSettings.EncodeOutputFile = fsAudioEncodeOutput.FileName;
                    audioStreamSettings.IntermediateFile = fsExistingAudioIntermediate.FileName;
                    audioStreamSettings.UseIntermediate = radAudioUseIF.Checked;
                }
            }
        }

        private void LoadVideoCodecSettings(EncodeSettingsProject proj)
        {
            IExecutionPlanSettings settings = null;
            if (proj.VideoCodecSettings.ContainsKey(proj.VideoCodecID))
                settings = proj.VideoCodecSettings[proj.VideoCodecID];
            else
            {
                for (int i = 0; i < CodecRepository.VideoCodecs.Length; i++)
                {
                    if (CodecRepository.VideoCodecs[i].CodecID == proj.VideoCodecID)
                    {
                        settings = CodecRepository.VideoCodecs[i].CreateExecutionPlanSettings();
                        break;
                    }
                }
            }
            if (settings != null)
                _videoCodecSettingsControl.LoadFromSettings(settings);
        }

        private void LoadAudioCodecSettings(EncodeSettingsProject proj)
        {
            if (proj.SelectedAudioStreamIndex == -1)
                return;

            IExecutionPlanSettings settings = null;
            EncodeSettingsProject.AudioStreamSettings audioSettings = proj.AudioStreams[proj.SelectedAudioStreamIndex];
            if (audioSettings.AudioCodecSettings.ContainsKey(proj.AudioCodecID))
                settings = audioSettings.AudioCodecSettings[proj.AudioCodecID];
            else
            {
                foreach (ICodec codec in CodecRepository.AudioCodecs)
                {
                    if (codec.CodecID == proj.AudioCodecID)
                    {
                        settings = codec.CreateExecutionPlanSettings();
                        break;
                    }
                }
            }
            if (settings != null)
            {
                fsAudioEncodeInput.FileName = audioSettings.EncodeInputFile;
                fsAudioEncodeOutput.FileName = audioSettings.EncodeOutputFile;
                fsExistingAudioIntermediate.FileName = audioSettings.IntermediateFile;
                radAudioEncode.Checked = !audioSettings.UseIntermediate;
                radAudioUseIF.Checked = audioSettings.UseIntermediate;
                _audioCodecSettingsControl.LoadFromSettings(settings);
            }
        }

        private void SaveProject()
        {
            _project.VideoCodecID = CodecRepository.VideoCodecs[cbxVideoCodec.SelectedIndex].CodecID;
            _project.AudioCodecID = CodecRepository.AudioCodecs[cbxAudioCodec.SelectedIndex].CodecID;
            _project.VideoIntermediateFile = fsExistingVideoIntermediate.FileName;
            _project.VideoEncodeInputFile = fsVideoEncodeInput.FileName;
            _project.VideoEncodeOutputFile = fsVideoEncodeOutput.FileName;
            _project.VideoUseIntermediate = radVideoUseIF.Checked;

            SaveCurrentVideoCodecSettings();
            SaveCurrentAudioCodecSettings();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            foreach (ICodec codec in CodecRepository.VideoCodecs)
                cbxVideoCodec.Items.Add(codec.Name);
            foreach (ICodec codec in CodecRepository.AudioCodecs)
                cbxAudioCodec.Items.Add(codec.Name);

            LoadProject(new EncodeSettingsProject());
        }

        private void GenerateExecutionPlans()
        {
            string lwmuxPath = System.IO.Directory.GetCurrentDirectory() + "\\lwmux.exe";
            string lwrerangePath = System.IO.Directory.GetCurrentDirectory() + "\\lwrerange.exe";
            string ffmpegPath = System.IO.Directory.GetCurrentDirectory() + "\\ffmpeg\\bin\\ffmpeg.exe";
            Dictionary<string, object> settings = new Dictionary<string, object>();

            settings["lwmuxPath"] = lwmuxPath;
            settings["lwrerangePath"] = lwrerangePath;
            settings["ffmpegPath"] = ffmpegPath;

            List<string> audioStreamPaths = new List<string>();
            string videoStreamPath = null;

            List<ExecutionPlan> ePlans = new List<ExecutionPlan>();
            if (!_project.VideoUseIntermediate)
            {
                EncodeSettingsProject proj = _project;

                settings["InputFile"] = _project.VideoEncodeInputFile;
                settings["OutputFile"] = _project.VideoEncodeOutputFile;
                _project.VideoCodecSettings[_project.VideoCodecID].CreateCommands(ePlans, settings, delegate()
                {
                    proj.VideoIntermediateFile = proj.VideoEncodeOutputFile;
                    proj.VideoUseIntermediate = true;
                });

                videoStreamPath = _project.VideoEncodeOutputFile;
            }
            else
            {
                videoStreamPath = _project.VideoIntermediateFile;
            }

            foreach (EncodeSettingsProject.AudioStreamSettings audioStream in _project.AudioStreams)
            {
                if (!audioStream.UseIntermediate)
                {
                    settings["InputFile"] = audioStream.EncodeInputFile;
                    settings["OutputFile"] = audioStream.EncodeOutputFile;

                    EncodeSettingsProject.AudioStreamSettings currentStream = audioStream;
                    audioStream.AudioCodecSettings[_project.AudioCodecID].CreateCommands(ePlans, settings, delegate()
                    {
                        audioStream.IntermediateFile = audioStream.EncodeOutputFile;
                        audioStream.UseIntermediate = true;
                    });

                    audioStreamPaths.Add(audioStream.EncodeOutputFile);
                }
                else
                {
                    audioStreamPaths.Add(audioStream.IntermediateFile);
                }
            }

            // Create finalize task
            List<string> muxArgs = new List<string>();
            muxArgs.Add("finalize");
            muxArgs.Add(videoStreamPath);
            muxArgs.Add(fsOutputFile.FileName);
            muxArgs.Add("10000");
            muxArgs.Add(audioStreamPaths.Count.ToString());
            muxArgs.AddRange(audioStreamPaths);

            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(lwmuxPath, muxArgs.ToArray());
                plan.AddStage(stage);
                ePlans.Add(plan);
            }

            TaskMonitor tm = new TaskMonitor(ePlans);
            tm.ShowDialog();

            LoadProject(_project);
        }

        private void btnGo_Click(object sender, EventArgs e)
        {
            SaveProject();

            GenerateExecutionPlans();
        }

        private void SaveProjectToFile(string path)
        {
            SaveProject();
            XmlDocument doc = _project.SaveToXml();
            doc.Save(path);
        }

        private void openSettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void saveSettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
        }

        private void fsVideoEncodeInput_Load(object sender, EventArgs e)
        {

        }

        private void btnAddAudioStream_Click(object sender, EventArgs e)
        {
            EncodeSettingsProject.AudioStreamSettings streamSettings = new EncodeSettingsProject.AudioStreamSettings();
            _project.AudioStreams.Add(streamSettings);
            cbxAudioStream.Items.Add("Stream " + (cbxAudioStream.Items.Count + 1).ToString());
            if (cbxAudioStream.Items.Count == 1)
                cbxAudioStream.SelectedIndex = 0;
        }

        private void RefreshAudioCodecSettings()
        {
            if (_project == null)
                return;

            if (_project.AudioStreams.Count != 0)
            {
                ICodec selectedCodec = null;
                foreach (ICodec codec in CodecRepository.AudioCodecs)
                {
                    if (codec.CodecID == _project.AudioCodecID)
                    {
                        selectedCodec = codec;
                        break;
                    }
                }

                if (selectedCodec != null)
                {
                    Control codecOptionsControl = selectedCodec.CreateCodecControl();

                    SetPanelContainedControl(pnlAudioCodecOptions, codecOptionsControl);

                    _audioCodecSettingsControl = (ICodecSettingsControl)codecOptionsControl;
                    LoadAudioCodecSettings(_project);
                }
            }
        }

        private void cbxAudioStream_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Save the current codec settings
            SaveCurrentAudioCodecSettings();

            int index = ((ComboBox)sender).SelectedIndex;
            if (_project != null)
                _project.SelectedAudioStreamIndex = index;

            gbxAudioIO.Visible = (index != -1);
            gbxAudioCodecOptions.Visible = (index != -1);

            RefreshAudioCodecSettings();
        }

        private void cbxAudioCodec_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Save the current codec settings
            SaveCurrentAudioCodecSettings();

            if (_project != null)
                _project.AudioCodecID = CodecRepository.AudioCodecs[((ComboBox)sender).SelectedIndex].CodecID;

            RefreshAudioCodecSettings();
        }

        private void fsAudioEncodeInput_FileChanged(object sender, EventArgs e)
        {
            if (fsAudioEncodeOutput.FileName == "")
            {
                FileSelector fs = (FileSelector)sender;
                string strippedFName = fs.FileName;
                int dotIndex = strippedFName.LastIndexOf('.');
                if (dotIndex != -1)
                    strippedFName = strippedFName.Substring(0, dotIndex);
                fsAudioEncodeOutput.FileName = strippedFName + ".lwia";
                fsExistingAudioIntermediate.FileName = strippedFName + ".lwia";
            }

            if (_project != null)
                radAudioEncode.Checked = true;
        }

        private void btnDeleteAudioStream_Click(object sender, EventArgs e)
        {
            ComboBox cbx = cbxAudioStream;
            int currentIndex = cbx.SelectedIndex;

            if (currentIndex == -1)
                return;

            cbx.SelectedIndex = -1;

            _project.AudioStreams.RemoveAt(currentIndex);

            // Remove the last item
            cbx.Items.RemoveAt(cbx.Items.Count - 1);
            if (currentIndex == cbx.Items.Count)
                currentIndex--;

            cbx.SelectedIndex = currentIndex;
        }

        private void btnAudioUseVideoSource_Click(object sender, EventArgs e)
        {
            fsAudioEncodeInput.FileName = fsVideoEncodeInput.FileName;
        }
    }
}
