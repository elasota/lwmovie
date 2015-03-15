namespace lwfe
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.pnlVideoCodecOptions = new System.Windows.Forms.Panel();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cbxVideoCodec = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.radVideoEncode = new System.Windows.Forms.RadioButton();
            this.radVideoUseIF = new System.Windows.Forms.RadioButton();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.gbxAudioIO = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.radAudioEncode = new System.Windows.Forms.RadioButton();
            this.radAudioUseIF = new System.Windows.Forms.RadioButton();
            this.gbxAudioCodecOptions = new System.Windows.Forms.GroupBox();
            this.pnlAudioCodecOptions = new System.Windows.Forms.Panel();
            this.cbxAudioCodec = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.btnDeleteAudioStream = new System.Windows.Forms.Button();
            this.btnAddAudioStream = new System.Windows.Forms.Button();
            this.cbxAudioStream = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.label4 = new System.Windows.Forms.Label();
            this.btnGo = new System.Windows.Forms.Button();
            this.mnsMenu = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveSettingsAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.btnAudioUseVideoSource = new System.Windows.Forms.Button();
            this.fsExistingVideoIntermediate = new lwfe.FileSelector();
            this.fsVideoEncodeOutput = new lwfe.FileSelector();
            this.fsVideoEncodeInput = new lwfe.FileSelector();
            this.fsExistingAudioIntermediate = new lwfe.FileSelector();
            this.fsAudioEncodeOutput = new lwfe.FileSelector();
            this.fsAudioEncodeInput = new lwfe.FileSelector();
            this.fsOutputFile = new lwfe.FileSelector();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.gbxAudioIO.SuspendLayout();
            this.gbxAudioCodecOptions.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.mnsMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(12, 27);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(533, 419);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.fsExistingVideoIntermediate);
            this.tabPage1.Controls.Add(this.groupBox1);
            this.tabPage1.Controls.Add(this.label6);
            this.tabPage1.Controls.Add(this.label5);
            this.tabPage1.Controls.Add(this.fsVideoEncodeOutput);
            this.tabPage1.Controls.Add(this.fsVideoEncodeInput);
            this.tabPage1.Controls.Add(this.cbxVideoCodec);
            this.tabPage1.Controls.Add(this.label3);
            this.tabPage1.Controls.Add(this.radVideoEncode);
            this.tabPage1.Controls.Add(this.radVideoUseIF);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(525, 393);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Video";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.pnlVideoCodecOptions);
            this.groupBox1.Location = new System.Drawing.Point(6, 157);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(513, 230);
            this.groupBox1.TabIndex = 9;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Codec Options";
            // 
            // pnlVideoCodecOptions
            // 
            this.pnlVideoCodecOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlVideoCodecOptions.Location = new System.Drawing.Point(6, 19);
            this.pnlVideoCodecOptions.Name = "pnlVideoCodecOptions";
            this.pnlVideoCodecOptions.Size = new System.Drawing.Size(501, 205);
            this.pnlVideoCodecOptions.TabIndex = 4;
            this.pnlVideoCodecOptions.Paint += new System.Windows.Forms.PaintEventHandler(this.pnlCodecOptions_Paint);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(31, 107);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(42, 13);
            this.label6.TabIndex = 8;
            this.label6.Text = "Output:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(38, 81);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(34, 13);
            this.label5.TabIndex = 7;
            this.label5.Text = "Input:";
            // 
            // cbxVideoCodec
            // 
            this.cbxVideoCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxVideoCodec.FormattingEnabled = true;
            this.cbxVideoCodec.Location = new System.Drawing.Point(78, 130);
            this.cbxVideoCodec.Name = "cbxVideoCodec";
            this.cbxVideoCodec.Size = new System.Drawing.Size(101, 21);
            this.cbxVideoCodec.TabIndex = 3;
            this.cbxVideoCodec.SelectedIndexChanged += new System.EventHandler(this.cbxVideoCodec_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(31, 133);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(41, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Codec:";
            // 
            // radVideoEncode
            // 
            this.radVideoEncode.AutoSize = true;
            this.radVideoEncode.Location = new System.Drawing.Point(6, 55);
            this.radVideoEncode.Name = "radVideoEncode";
            this.radVideoEncode.Size = new System.Drawing.Size(100, 17);
            this.radVideoEncode.TabIndex = 1;
            this.radVideoEncode.TabStop = true;
            this.radVideoEncode.Text = "Encode a video";
            this.radVideoEncode.UseVisualStyleBackColor = true;
            // 
            // radVideoUseIF
            // 
            this.radVideoUseIF.AutoSize = true;
            this.radVideoUseIF.Location = new System.Drawing.Point(6, 6);
            this.radVideoUseIF.Name = "radVideoUseIF";
            this.radVideoUseIF.Size = new System.Drawing.Size(173, 17);
            this.radVideoUseIF.TabIndex = 0;
            this.radVideoUseIF.TabStop = true;
            this.radVideoUseIF.Text = "Use an existing intermediate file";
            this.radVideoUseIF.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.gbxAudioIO);
            this.tabPage2.Controls.Add(this.gbxAudioCodecOptions);
            this.tabPage2.Controls.Add(this.cbxAudioCodec);
            this.tabPage2.Controls.Add(this.label2);
            this.tabPage2.Controls.Add(this.btnDeleteAudioStream);
            this.tabPage2.Controls.Add(this.btnAddAudioStream);
            this.tabPage2.Controls.Add(this.cbxAudioStream);
            this.tabPage2.Controls.Add(this.label1);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(525, 393);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Audio";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // gbxAudioIO
            // 
            this.gbxAudioIO.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.gbxAudioIO.Controls.Add(this.btnAudioUseVideoSource);
            this.gbxAudioIO.Controls.Add(this.fsExistingAudioIntermediate);
            this.gbxAudioIO.Controls.Add(this.label7);
            this.gbxAudioIO.Controls.Add(this.label8);
            this.gbxAudioIO.Controls.Add(this.fsAudioEncodeOutput);
            this.gbxAudioIO.Controls.Add(this.fsAudioEncodeInput);
            this.gbxAudioIO.Controls.Add(this.radAudioEncode);
            this.gbxAudioIO.Controls.Add(this.radAudioUseIF);
            this.gbxAudioIO.Location = new System.Drawing.Point(6, 60);
            this.gbxAudioIO.Name = "gbxAudioIO";
            this.gbxAudioIO.Size = new System.Drawing.Size(513, 178);
            this.gbxAudioIO.TabIndex = 7;
            this.gbxAudioIO.TabStop = false;
            this.gbxAudioIO.Text = "Input/Output";
            this.gbxAudioIO.Visible = false;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(31, 149);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(42, 13);
            this.label7.TabIndex = 23;
            this.label7.Text = "Output:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(38, 123);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(34, 13);
            this.label8.TabIndex = 22;
            this.label8.Text = "Input:";
            // 
            // radAudioEncode
            // 
            this.radAudioEncode.AutoSize = true;
            this.radAudioEncode.Location = new System.Drawing.Point(6, 68);
            this.radAudioEncode.Name = "radAudioEncode";
            this.radAudioEncode.Size = new System.Drawing.Size(140, 17);
            this.radAudioEncode.TabIndex = 19;
            this.radAudioEncode.TabStop = true;
            this.radAudioEncode.Text = "Encode an audio stream";
            this.radAudioEncode.UseVisualStyleBackColor = true;
            // 
            // radAudioUseIF
            // 
            this.radAudioUseIF.AutoSize = true;
            this.radAudioUseIF.Location = new System.Drawing.Point(6, 19);
            this.radAudioUseIF.Name = "radAudioUseIF";
            this.radAudioUseIF.Size = new System.Drawing.Size(173, 17);
            this.radAudioUseIF.TabIndex = 18;
            this.radAudioUseIF.TabStop = true;
            this.radAudioUseIF.Text = "Use an existing intermediate file";
            this.radAudioUseIF.UseVisualStyleBackColor = true;
            // 
            // gbxAudioCodecOptions
            // 
            this.gbxAudioCodecOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.gbxAudioCodecOptions.Controls.Add(this.pnlAudioCodecOptions);
            this.gbxAudioCodecOptions.Location = new System.Drawing.Point(6, 244);
            this.gbxAudioCodecOptions.Name = "gbxAudioCodecOptions";
            this.gbxAudioCodecOptions.Size = new System.Drawing.Size(513, 143);
            this.gbxAudioCodecOptions.TabIndex = 6;
            this.gbxAudioCodecOptions.TabStop = false;
            this.gbxAudioCodecOptions.Text = "Codec Options";
            this.gbxAudioCodecOptions.Visible = false;
            // 
            // pnlAudioCodecOptions
            // 
            this.pnlAudioCodecOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlAudioCodecOptions.Location = new System.Drawing.Point(6, 19);
            this.pnlAudioCodecOptions.Name = "pnlAudioCodecOptions";
            this.pnlAudioCodecOptions.Size = new System.Drawing.Size(501, 118);
            this.pnlAudioCodecOptions.TabIndex = 4;
            // 
            // cbxAudioCodec
            // 
            this.cbxAudioCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxAudioCodec.FormattingEnabled = true;
            this.cbxAudioCodec.Location = new System.Drawing.Point(55, 6);
            this.cbxAudioCodec.Name = "cbxAudioCodec";
            this.cbxAudioCodec.Size = new System.Drawing.Size(121, 21);
            this.cbxAudioCodec.TabIndex = 5;
            this.cbxAudioCodec.SelectedIndexChanged += new System.EventHandler(this.cbxAudioCodec_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Codec:";
            // 
            // btnDeleteAudioStream
            // 
            this.btnDeleteAudioStream.Location = new System.Drawing.Point(207, 33);
            this.btnDeleteAudioStream.Name = "btnDeleteAudioStream";
            this.btnDeleteAudioStream.Size = new System.Drawing.Size(75, 21);
            this.btnDeleteAudioStream.TabIndex = 3;
            this.btnDeleteAudioStream.Text = "Delete";
            this.btnDeleteAudioStream.UseVisualStyleBackColor = true;
            this.btnDeleteAudioStream.Click += new System.EventHandler(this.btnDeleteAudioStream_Click);
            // 
            // btnAddAudioStream
            // 
            this.btnAddAudioStream.Location = new System.Drawing.Point(182, 33);
            this.btnAddAudioStream.Name = "btnAddAudioStream";
            this.btnAddAudioStream.Size = new System.Drawing.Size(19, 21);
            this.btnAddAudioStream.TabIndex = 2;
            this.btnAddAudioStream.Text = "+";
            this.btnAddAudioStream.UseVisualStyleBackColor = true;
            this.btnAddAudioStream.Click += new System.EventHandler(this.btnAddAudioStream_Click);
            // 
            // cbxAudioStream
            // 
            this.cbxAudioStream.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxAudioStream.FormattingEnabled = true;
            this.cbxAudioStream.Location = new System.Drawing.Point(55, 33);
            this.cbxAudioStream.Name = "cbxAudioStream";
            this.cbxAudioStream.Size = new System.Drawing.Size(121, 21);
            this.cbxAudioStream.TabIndex = 1;
            this.cbxAudioStream.SelectedIndexChanged += new System.EventHandler(this.cbxAudioStream_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 36);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(43, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Stream:";
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.label4);
            this.tabPage3.Controls.Add(this.fsOutputFile);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(525, 393);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Output";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 13);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(58, 13);
            this.label4.TabIndex = 0;
            this.label4.Text = "Output file:";
            // 
            // btnGo
            // 
            this.btnGo.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnGo.Location = new System.Drawing.Point(421, 452);
            this.btnGo.Name = "btnGo";
            this.btnGo.Size = new System.Drawing.Size(124, 23);
            this.btnGo.TabIndex = 1;
            this.btnGo.Text = "Begin Processing";
            this.btnGo.UseVisualStyleBackColor = true;
            this.btnGo.Click += new System.EventHandler(this.btnGo_Click);
            // 
            // mnsMenu
            // 
            this.mnsMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.mnsMenu.Location = new System.Drawing.Point(0, 0);
            this.mnsMenu.Name = "mnsMenu";
            this.mnsMenu.Size = new System.Drawing.Size(557, 24);
            this.mnsMenu.TabIndex = 2;
            this.mnsMenu.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openSettingsToolStripMenuItem,
            this.saveSettingsToolStripMenuItem,
            this.saveSettingsAsToolStripMenuItem,
            this.toolStripMenuItem2,
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openSettingsToolStripMenuItem
            // 
            this.openSettingsToolStripMenuItem.Name = "openSettingsToolStripMenuItem";
            this.openSettingsToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.openSettingsToolStripMenuItem.Text = "Open Settings...";
            this.openSettingsToolStripMenuItem.Click += new System.EventHandler(this.openSettingsToolStripMenuItem_Click);
            // 
            // saveSettingsToolStripMenuItem
            // 
            this.saveSettingsToolStripMenuItem.Name = "saveSettingsToolStripMenuItem";
            this.saveSettingsToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.saveSettingsToolStripMenuItem.Text = "Save Settings";
            this.saveSettingsToolStripMenuItem.Click += new System.EventHandler(this.saveSettingsToolStripMenuItem_Click);
            // 
            // saveSettingsAsToolStripMenuItem
            // 
            this.saveSettingsAsToolStripMenuItem.Name = "saveSettingsAsToolStripMenuItem";
            this.saveSettingsAsToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.saveSettingsAsToolStripMenuItem.Text = "Save Settings As...";
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(165, 6);
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            // 
            // btnAudioUseVideoSource
            // 
            this.btnAudioUseVideoSource.Location = new System.Drawing.Point(78, 91);
            this.btnAudioUseVideoSource.Name = "btnAudioUseVideoSource";
            this.btnAudioUseVideoSource.Size = new System.Drawing.Size(161, 23);
            this.btnAudioUseVideoSource.TabIndex = 25;
            this.btnAudioUseVideoSource.Text = "Use Video Source As Input";
            this.btnAudioUseVideoSource.UseVisualStyleBackColor = true;
            this.btnAudioUseVideoSource.Click += new System.EventHandler(this.btnAudioUseVideoSource_Click);
            // 
            // fsExistingVideoIntermediate
            // 
            this.fsExistingVideoIntermediate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsExistingVideoIntermediate.DefaultDirectory = null;
            this.fsExistingVideoIntermediate.DefaultFilterIndex = 0;
            this.fsExistingVideoIntermediate.FileName = "";
            this.fsExistingVideoIntermediate.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Open;
            this.fsExistingVideoIntermediate.FileTypeFilters = "lwmovie Intermediate Video (*lwiv)|*.lwiv";
            this.fsExistingVideoIntermediate.Location = new System.Drawing.Point(78, 29);
            this.fsExistingVideoIntermediate.Name = "fsExistingVideoIntermediate";
            this.fsExistingVideoIntermediate.Size = new System.Drawing.Size(445, 20);
            this.fsExistingVideoIntermediate.TabIndex = 10;
            // 
            // fsVideoEncodeOutput
            // 
            this.fsVideoEncodeOutput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsVideoEncodeOutput.DefaultDirectory = null;
            this.fsVideoEncodeOutput.DefaultFilterIndex = 0;
            this.fsVideoEncodeOutput.FileName = "";
            this.fsVideoEncodeOutput.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Save;
            this.fsVideoEncodeOutput.FileTypeFilters = "lwmovie Intermediate Video (*.lwiv)|*.lwiv";
            this.fsVideoEncodeOutput.Location = new System.Drawing.Point(78, 104);
            this.fsVideoEncodeOutput.Name = "fsVideoEncodeOutput";
            this.fsVideoEncodeOutput.Size = new System.Drawing.Size(445, 20);
            this.fsVideoEncodeOutput.TabIndex = 6;
            // 
            // fsVideoEncodeInput
            // 
            this.fsVideoEncodeInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsVideoEncodeInput.DefaultDirectory = null;
            this.fsVideoEncodeInput.DefaultFilterIndex = 0;
            this.fsVideoEncodeInput.FileName = "";
            this.fsVideoEncodeInput.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Open;
            this.fsVideoEncodeInput.FileTypeFilters = "Input Files (*.avi,*.mp4)|*.avi;*.mp4";
            this.fsVideoEncodeInput.Location = new System.Drawing.Point(78, 78);
            this.fsVideoEncodeInput.Name = "fsVideoEncodeInput";
            this.fsVideoEncodeInput.Size = new System.Drawing.Size(445, 20);
            this.fsVideoEncodeInput.TabIndex = 5;
            this.fsVideoEncodeInput.FileChanged += new System.EventHandler(this.fsVideoEncodeInput_FileChanged);
            // 
            // fsExistingAudioIntermediate
            // 
            this.fsExistingAudioIntermediate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsExistingAudioIntermediate.DefaultDirectory = null;
            this.fsExistingAudioIntermediate.DefaultFilterIndex = 0;
            this.fsExistingAudioIntermediate.FileName = "";
            this.fsExistingAudioIntermediate.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Open;
            this.fsExistingAudioIntermediate.FileTypeFilters = "lwmovie Intermediate Audio (*.lwia)|*.lwia";
            this.fsExistingAudioIntermediate.Location = new System.Drawing.Point(78, 42);
            this.fsExistingAudioIntermediate.Name = "fsExistingAudioIntermediate";
            this.fsExistingAudioIntermediate.Size = new System.Drawing.Size(429, 20);
            this.fsExistingAudioIntermediate.TabIndex = 24;
            // 
            // fsAudioEncodeOutput
            // 
            this.fsAudioEncodeOutput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsAudioEncodeOutput.DefaultDirectory = null;
            this.fsAudioEncodeOutput.DefaultFilterIndex = 0;
            this.fsAudioEncodeOutput.FileName = "";
            this.fsAudioEncodeOutput.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Save;
            this.fsAudioEncodeOutput.FileTypeFilters = "lwmovie Intermediate Audio (*.lwia)|*.lwia";
            this.fsAudioEncodeOutput.Location = new System.Drawing.Point(78, 146);
            this.fsAudioEncodeOutput.Name = "fsAudioEncodeOutput";
            this.fsAudioEncodeOutput.Size = new System.Drawing.Size(429, 20);
            this.fsAudioEncodeOutput.TabIndex = 21;
            // 
            // fsAudioEncodeInput
            // 
            this.fsAudioEncodeInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsAudioEncodeInput.DefaultDirectory = null;
            this.fsAudioEncodeInput.DefaultFilterIndex = 0;
            this.fsAudioEncodeInput.FileName = "";
            this.fsAudioEncodeInput.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Open;
            this.fsAudioEncodeInput.FileTypeFilters = "Input Files (*.avi,*.mp4,*.wav,*.mp2)|*.avi;*.mp4;*.wav;*.mp2";
            this.fsAudioEncodeInput.Location = new System.Drawing.Point(78, 120);
            this.fsAudioEncodeInput.Name = "fsAudioEncodeInput";
            this.fsAudioEncodeInput.Size = new System.Drawing.Size(429, 20);
            this.fsAudioEncodeInput.TabIndex = 20;
            this.fsAudioEncodeInput.FileChanged += new System.EventHandler(this.fsAudioEncodeInput_FileChanged);
            // 
            // fsOutputFile
            // 
            this.fsOutputFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fsOutputFile.DefaultDirectory = null;
            this.fsOutputFile.DefaultFilterIndex = 0;
            this.fsOutputFile.FileName = "";
            this.fsOutputFile.FileSelectionMode = lwfe.FileSelector.EFileSelectionMode.Save;
            this.fsOutputFile.FileTypeFilters = "lwmovie files (*.lwmv)|*.lwmv|All files (*.*)|*.*";
            this.fsOutputFile.Location = new System.Drawing.Point(70, 9);
            this.fsOutputFile.Name = "fsOutputFile";
            this.fsOutputFile.Size = new System.Drawing.Size(449, 20);
            this.fsOutputFile.TabIndex = 1;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(557, 487);
            this.Controls.Add(this.btnGo);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.mnsMenu);
            this.Name = "MainForm";
            this.Text = "lwmovie Encoder Front-End";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.gbxAudioIO.ResumeLayout(false);
            this.gbxAudioIO.PerformLayout();
            this.gbxAudioCodecOptions.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.mnsMenu.ResumeLayout(false);
            this.mnsMenu.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.ComboBox cbxAudioCodec;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnDeleteAudioStream;
        private System.Windows.Forms.Button btnAddAudioStream;
        private System.Windows.Forms.ComboBox cbxAudioStream;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbxVideoCodec;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.RadioButton radVideoEncode;
        private System.Windows.Forms.RadioButton radVideoUseIF;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.Label label4;
        private FileSelector fsOutputFile;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private FileSelector fsVideoEncodeOutput;
        private FileSelector fsVideoEncodeInput;
        private System.Windows.Forms.Button btnGo;
        private System.Windows.Forms.MenuStrip mnsMenu;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openSettingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveSettingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveSettingsAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Panel pnlVideoCodecOptions;
        private FileSelector fsExistingVideoIntermediate;
        private System.Windows.Forms.GroupBox gbxAudioCodecOptions;
        private System.Windows.Forms.Panel pnlAudioCodecOptions;
        private System.Windows.Forms.GroupBox gbxAudioIO;
        private FileSelector fsExistingAudioIntermediate;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private FileSelector fsAudioEncodeOutput;
        private FileSelector fsAudioEncodeInput;
        private System.Windows.Forms.RadioButton radAudioEncode;
        private System.Windows.Forms.RadioButton radAudioUseIF;
        private System.Windows.Forms.Button btnAudioUseVideoSource;
    }
}

