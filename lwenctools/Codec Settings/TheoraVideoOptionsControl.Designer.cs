namespace lwenctools
{
    partial class TheoraVideoOptionsControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.txtBitrate = new System.Windows.Forms.TextBox();
            this.lblBitrateMaxSuffix = new System.Windows.Forms.Label();
            this.cbxBitratePresets = new System.Windows.Forms.ComboBox();
            this.radUseQuality = new System.Windows.Forms.RadioButton();
            this.lblQualityLevel = new System.Windows.Forms.Label();
            this.radUseCbr = new System.Windows.Forms.RadioButton();
            this.tbQuality = new System.Windows.Forms.TrackBar();
            this.label13 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.label15 = new System.Windows.Forms.Label();
            this.chkFullRange = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.chkTwoPass = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.chkKeyFrames = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.nbKeyRate = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.chkFullPrecisionChroma = new System.Windows.Forms.CheckBox();
            this.nbBacklogSize = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.cbxEncoderSpeed = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.tbQuality)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nbKeyRate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nbBacklogSize)).BeginInit();
            this.SuspendLayout();
            // 
            // txtBitrate
            // 
            this.txtBitrate.Location = new System.Drawing.Point(165, 243);
            this.txtBitrate.Name = "txtBitrate";
            this.txtBitrate.Size = new System.Drawing.Size(51, 20);
            this.txtBitrate.TabIndex = 9;
            // 
            // lblBitrateMaxSuffix
            // 
            this.lblBitrateMaxSuffix.AutoSize = true;
            this.lblBitrateMaxSuffix.Location = new System.Drawing.Point(222, 246);
            this.lblBitrateMaxSuffix.Name = "lblBitrateMaxSuffix";
            this.lblBitrateMaxSuffix.Size = new System.Drawing.Size(30, 13);
            this.lblBitrateMaxSuffix.TabIndex = 28;
            this.lblBitrateMaxSuffix.Text = "kbps";
            // 
            // cbxBitratePresets
            // 
            this.cbxBitratePresets.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitratePresets.FormattingEnabled = true;
            this.cbxBitratePresets.Items.AddRange(new object[] {
            "Custom..."});
            this.cbxBitratePresets.Location = new System.Drawing.Point(38, 243);
            this.cbxBitratePresets.Name = "cbxBitratePresets";
            this.cbxBitratePresets.Size = new System.Drawing.Size(121, 21);
            this.cbxBitratePresets.TabIndex = 8;
            this.cbxBitratePresets.SelectedIndexChanged += new System.EventHandler(this.cbxBitratePresets_SelectedIndexChanged);
            // 
            // radUseQuality
            // 
            this.radUseQuality.AutoSize = true;
            this.radUseQuality.Checked = true;
            this.radUseQuality.Location = new System.Drawing.Point(6, 146);
            this.radUseQuality.Name = "radUseQuality";
            this.radUseQuality.Size = new System.Drawing.Size(125, 17);
            this.radUseQuality.TabIndex = 6;
            this.radUseQuality.TabStop = true;
            this.radUseQuality.Text = "Constant quality level";
            this.radUseQuality.UseVisualStyleBackColor = true;
            // 
            // lblQualityLevel
            // 
            this.lblQualityLevel.AutoSize = true;
            this.lblQualityLevel.Location = new System.Drawing.Point(12, 169);
            this.lblQualityLevel.Name = "lblQualityLevel";
            this.lblQualityLevel.Size = new System.Drawing.Size(19, 13);
            this.lblQualityLevel.TabIndex = 39;
            this.lblQualityLevel.Text = "31";
            // 
            // radUseCbr
            // 
            this.radUseCbr.AutoSize = true;
            this.radUseCbr.Location = new System.Drawing.Point(6, 220);
            this.radUseCbr.Name = "radUseCbr";
            this.radUseCbr.Size = new System.Drawing.Size(85, 17);
            this.radUseCbr.TabIndex = 36;
            this.radUseCbr.TabStop = true;
            this.radUseCbr.Text = "Fixed bit rate";
            this.radUseCbr.UseVisualStyleBackColor = true;
            // 
            // tbQuality
            // 
            this.tbQuality.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tbQuality.BackColor = System.Drawing.SystemColors.Window;
            this.tbQuality.Location = new System.Drawing.Point(37, 169);
            this.tbQuality.Maximum = 63;
            this.tbQuality.Minimum = 1;
            this.tbQuality.Name = "tbQuality";
            this.tbQuality.Size = new System.Drawing.Size(370, 45);
            this.tbQuality.TabIndex = 7;
            this.tbQuality.Value = 1;
            this.tbQuality.ValueChanged += new System.EventHandler(this.tbQuality_ValueChanged);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(34, 201);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(41, 13);
            this.label13.TabIndex = 40;
            this.label13.Text = "Lowest";
            // 
            // label12
            // 
            this.label12.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(364, 201);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(43, 13);
            this.label12.TabIndex = 41;
            this.label12.Text = "Highest";
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(3, 56);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(81, 13);
            this.label15.TabIndex = 47;
            this.label15.Text = "Full-range YUV:";
            // 
            // chkFullRange
            // 
            this.chkFullRange.AutoSize = true;
            this.chkFullRange.Location = new System.Drawing.Point(120, 55);
            this.chkFullRange.Name = "chkFullRange";
            this.chkFullRange.Size = new System.Drawing.Size(65, 17);
            this.chkFullRange.TabIndex = 2;
            this.chkFullRange.Text = "Enabled";
            this.chkFullRange.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 10);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(88, 13);
            this.label3.TabIndex = 43;
            this.label3.Text = "2-pass encoding:";
            // 
            // chkTwoPass
            // 
            this.chkTwoPass.AutoSize = true;
            this.chkTwoPass.Location = new System.Drawing.Point(120, 9);
            this.chkTwoPass.Name = "chkTwoPass";
            this.chkTwoPass.Size = new System.Drawing.Size(65, 17);
            this.chkTwoPass.TabIndex = 0;
            this.chkTwoPass.Text = "Enabled";
            this.chkTwoPass.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 79);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(62, 13);
            this.label1.TabIndex = 49;
            this.label1.Text = "Key frames:";
            // 
            // chkKeyFrames
            // 
            this.chkKeyFrames.AutoSize = true;
            this.chkKeyFrames.Location = new System.Drawing.Point(120, 78);
            this.chkKeyFrames.Name = "chkKeyFrames";
            this.chkKeyFrames.Size = new System.Drawing.Size(65, 17);
            this.chkKeyFrames.TabIndex = 3;
            this.chkKeyFrames.Text = "Enabled";
            this.chkKeyFrames.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(32, 103);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(83, 13);
            this.label2.TabIndex = 50;
            this.label2.Text = "Key frame every";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(223, 103);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(38, 13);
            this.label4.TabIndex = 51;
            this.label4.Text = "frames";
            // 
            // nbKeyRate
            // 
            this.nbKeyRate.Location = new System.Drawing.Point(120, 101);
            this.nbKeyRate.Maximum = new decimal(new int[] {
            300,
            0,
            0,
            0});
            this.nbKeyRate.Name = "nbKeyRate";
            this.nbKeyRate.Size = new System.Drawing.Size(97, 20);
            this.nbKeyRate.TabIndex = 4;
            this.nbKeyRate.Value = new decimal(new int[] {
            64,
            0,
            0,
            0});
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 33);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(109, 13);
            this.label5.TabIndex = 55;
            this.label5.Text = "Full-precision chroma:";
            // 
            // chkFullPrecisionChroma
            // 
            this.chkFullPrecisionChroma.AutoSize = true;
            this.chkFullPrecisionChroma.Location = new System.Drawing.Point(120, 32);
            this.chkFullPrecisionChroma.Name = "chkFullPrecisionChroma";
            this.chkFullPrecisionChroma.Size = new System.Drawing.Size(65, 17);
            this.chkFullPrecisionChroma.TabIndex = 1;
            this.chkFullPrecisionChroma.Text = "Enabled";
            this.chkFullPrecisionChroma.UseVisualStyleBackColor = true;
            // 
            // nbBacklogSize
            // 
            this.nbBacklogSize.Location = new System.Drawing.Point(108, 269);
            this.nbBacklogSize.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.nbBacklogSize.Name = "nbBacklogSize";
            this.nbBacklogSize.Size = new System.Drawing.Size(51, 20);
            this.nbBacklogSize.TabIndex = 10;
            this.nbBacklogSize.Value = new decimal(new int[] {
            12,
            0,
            0,
            0});
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(35, 271);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(67, 13);
            this.label7.TabIndex = 56;
            this.label7.Text = "Backlog size";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(165, 271);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(38, 13);
            this.label8.TabIndex = 58;
            this.label8.Text = "frames";
            // 
            // cbxEncoderSpeed
            // 
            this.cbxEncoderSpeed.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxEncoderSpeed.FormattingEnabled = true;
            this.cbxEncoderSpeed.Items.AddRange(new object[] {
            "Best",
            "Good",
            "Fast",
            "Faster",
            "Fastest"});
            this.cbxEncoderSpeed.Location = new System.Drawing.Point(279, 7);
            this.cbxEncoderSpeed.Name = "cbxEncoderSpeed";
            this.cbxEncoderSpeed.Size = new System.Drawing.Size(121, 21);
            this.cbxEncoderSpeed.TabIndex = 5;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(191, 10);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(82, 13);
            this.label6.TabIndex = 61;
            this.label6.Text = "Encoder speed:";
            // 
            // TheoraVideoOptionsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.cbxEncoderSpeed);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.nbBacklogSize);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.chkFullPrecisionChroma);
            this.Controls.Add(this.nbKeyRate);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.chkKeyFrames);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.chkFullRange);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.chkTwoPass);
            this.Controls.Add(this.label13);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.radUseQuality);
            this.Controls.Add(this.lblQualityLevel);
            this.Controls.Add(this.radUseCbr);
            this.Controls.Add(this.tbQuality);
            this.Controls.Add(this.txtBitrate);
            this.Controls.Add(this.lblBitrateMaxSuffix);
            this.Controls.Add(this.cbxBitratePresets);
            this.Name = "TheoraVideoOptionsControl";
            this.Size = new System.Drawing.Size(407, 296);
            ((System.ComponentModel.ISupportInitialize)(this.tbQuality)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nbKeyRate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nbBacklogSize)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtBitrate;
        private System.Windows.Forms.Label lblBitrateMaxSuffix;
        private System.Windows.Forms.ComboBox cbxBitratePresets;
        private System.Windows.Forms.RadioButton radUseQuality;
        private System.Windows.Forms.Label lblQualityLevel;
        private System.Windows.Forms.RadioButton radUseCbr;
        private System.Windows.Forms.TrackBar tbQuality;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.CheckBox chkFullRange;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox chkTwoPass;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox chkKeyFrames;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown nbKeyRate;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox chkFullPrecisionChroma;
        private System.Windows.Forms.NumericUpDown nbBacklogSize;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox cbxEncoderSpeed;
        private System.Windows.Forms.Label label6;
    }
}
