namespace lwenctools
{
    partial class MPEG2VideoOptionsControl
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
            this.label1 = new System.Windows.Forms.Label();
            this.txtSlices = new System.Windows.Forms.TextBox();
            this.chkTwoPass = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.cbxBFrameGroupSize = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbxBStrategy = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.cbxMotionEstimation = new System.Windows.Forms.ComboBox();
            this.chkAutoRD = new System.Windows.Forms.CheckBox();
            this.label6 = new System.Windows.Forms.Label();
            this.lblQualityLevel = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.txtBitrateBufferSize = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.txtBitrateMax = new System.Windows.Forms.TextBox();
            this.lblBitrateMaxSuffix = new System.Windows.Forms.Label();
            this.cbxBitrateMaxPresets = new System.Windows.Forms.ComboBox();
            this.txtBitrateMin = new System.Windows.Forms.TextBox();
            this.lblBitrateMinSuffix = new System.Windows.Forms.Label();
            this.cbxBitrateMinPresets = new System.Windows.Forms.ComboBox();
            this.radUseCbr = new System.Windows.Forms.RadioButton();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.tbQuality = new System.Windows.Forms.TrackBar();
            this.radUseQuality = new System.Windows.Forms.RadioButton();
            this.chkFullRange = new System.Windows.Forms.CheckBox();
            this.label15 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.tbQuality)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 6);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(83, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Maximum slices:";
            // 
            // txtSlices
            // 
            this.txtSlices.Location = new System.Drawing.Point(120, 3);
            this.txtSlices.Name = "txtSlices";
            this.txtSlices.Size = new System.Drawing.Size(79, 20);
            this.txtSlices.TabIndex = 0;
            this.txtSlices.Text = "8";
            // 
            // chkTwoPass
            // 
            this.chkTwoPass.AutoSize = true;
            this.chkTwoPass.Location = new System.Drawing.Point(120, 32);
            this.chkTwoPass.Name = "chkTwoPass";
            this.chkTwoPass.Size = new System.Drawing.Size(65, 17);
            this.chkTwoPass.TabIndex = 1;
            this.chkTwoPass.Text = "Enabled";
            this.chkTwoPass.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(212, 6);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(97, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "B-frame group size:";
            // 
            // cbxBFrameGroupSize
            // 
            this.cbxBFrameGroupSize.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbxBFrameGroupSize.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBFrameGroupSize.FormattingEnabled = true;
            this.cbxBFrameGroupSize.Items.AddRange(new object[] {
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "10",
            "11",
            "12",
            "13",
            "14",
            "15",
            "16"});
            this.cbxBFrameGroupSize.Location = new System.Drawing.Point(329, 3);
            this.cbxBFrameGroupSize.Name = "cbxBFrameGroupSize";
            this.cbxBFrameGroupSize.Size = new System.Drawing.Size(121, 21);
            this.cbxBFrameGroupSize.TabIndex = 4;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 33);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(88, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "2-pass encoding:";
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(212, 33);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(91, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "B-frame selection:";
            // 
            // cbxBStrategy
            // 
            this.cbxBStrategy.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbxBStrategy.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBStrategy.FormattingEnabled = true;
            this.cbxBStrategy.Items.AddRange(new object[] {
            "Fixed",
            "Auto - Fast",
            "Auto - High quality"});
            this.cbxBStrategy.Location = new System.Drawing.Point(329, 30);
            this.cbxBStrategy.Name = "cbxBStrategy";
            this.cbxBStrategy.Size = new System.Drawing.Size(121, 21);
            this.cbxBStrategy.TabIndex = 5;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(212, 62);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(107, 13);
            this.label5.TabIndex = 8;
            this.label5.Text = "Motion search range:";
            // 
            // cbxMotionEstimation
            // 
            this.cbxMotionEstimation.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbxMotionEstimation.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxMotionEstimation.FormattingEnabled = true;
            this.cbxMotionEstimation.Location = new System.Drawing.Point(329, 57);
            this.cbxMotionEstimation.Name = "cbxMotionEstimation";
            this.cbxMotionEstimation.Size = new System.Drawing.Size(121, 21);
            this.cbxMotionEstimation.TabIndex = 6;
            // 
            // chkAutoRD
            // 
            this.chkAutoRD.AutoSize = true;
            this.chkAutoRD.Location = new System.Drawing.Point(120, 61);
            this.chkAutoRD.Name = "chkAutoRD";
            this.chkAutoRD.Size = new System.Drawing.Size(65, 17);
            this.chkAutoRD.TabIndex = 2;
            this.chkAutoRD.Text = "Enabled";
            this.chkAutoRD.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(3, 62);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(112, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "High-quality encoding:";
            // 
            // lblQualityLevel
            // 
            this.lblQualityLevel.AutoSize = true;
            this.lblQualityLevel.Location = new System.Drawing.Point(12, 42);
            this.lblQualityLevel.Name = "lblQualityLevel";
            this.lblQualityLevel.Size = new System.Drawing.Size(19, 13);
            this.lblQualityLevel.TabIndex = 31;
            this.lblQualityLevel.Text = "31";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(404, 120);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(32, 13);
            this.label7.TabIndex = 30;
            this.label7.Text = "bytes";
            // 
            // txtBitrateBufferSize
            // 
            this.txtBitrateBufferSize.Location = new System.Drawing.Point(298, 117);
            this.txtBitrateBufferSize.Name = "txtBitrateBufferSize";
            this.txtBitrateBufferSize.Size = new System.Drawing.Size(100, 20);
            this.txtBitrateBufferSize.TabIndex = 29;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(233, 120);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(59, 13);
            this.label8.TabIndex = 28;
            this.label8.Text = "Buffer size:";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(3, 270);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(86, 13);
            this.label9.TabIndex = 27;
            this.label9.Text = "Maximum bit rate";
            // 
            // txtBitrateMax
            // 
            this.txtBitrateMax.Location = new System.Drawing.Point(133, 287);
            this.txtBitrateMax.Name = "txtBitrateMax";
            this.txtBitrateMax.Size = new System.Drawing.Size(51, 20);
            this.txtBitrateMax.TabIndex = 9;
            // 
            // lblBitrateMaxSuffix
            // 
            this.lblBitrateMaxSuffix.AutoSize = true;
            this.lblBitrateMaxSuffix.Location = new System.Drawing.Point(190, 290);
            this.lblBitrateMaxSuffix.Name = "lblBitrateMaxSuffix";
            this.lblBitrateMaxSuffix.Size = new System.Drawing.Size(30, 13);
            this.lblBitrateMaxSuffix.TabIndex = 25;
            this.lblBitrateMaxSuffix.Text = "kbps";
            // 
            // cbxBitrateMaxPresets
            // 
            this.cbxBitrateMaxPresets.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitrateMaxPresets.FormattingEnabled = true;
            this.cbxBitrateMaxPresets.Items.AddRange(new object[] {
            "Custom..."});
            this.cbxBitrateMaxPresets.Location = new System.Drawing.Point(6, 286);
            this.cbxBitrateMaxPresets.Name = "cbxBitrateMaxPresets";
            this.cbxBitrateMaxPresets.Size = new System.Drawing.Size(121, 21);
            this.cbxBitrateMaxPresets.TabIndex = 8;
            this.cbxBitrateMaxPresets.SelectedIndexChanged += new System.EventHandler(this.cbxBitrateMaxPresets_SelectedIndexChanged);
            // 
            // txtBitrateMin
            // 
            this.txtBitrateMin.Location = new System.Drawing.Point(133, 117);
            this.txtBitrateMin.Name = "txtBitrateMin";
            this.txtBitrateMin.Size = new System.Drawing.Size(51, 20);
            this.txtBitrateMin.TabIndex = 23;
            // 
            // lblBitrateMinSuffix
            // 
            this.lblBitrateMinSuffix.AutoSize = true;
            this.lblBitrateMinSuffix.Location = new System.Drawing.Point(190, 120);
            this.lblBitrateMinSuffix.Name = "lblBitrateMinSuffix";
            this.lblBitrateMinSuffix.Size = new System.Drawing.Size(30, 13);
            this.lblBitrateMinSuffix.TabIndex = 22;
            this.lblBitrateMinSuffix.Text = "kbps";
            // 
            // cbxBitrateMinPresets
            // 
            this.cbxBitrateMinPresets.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitrateMinPresets.FormattingEnabled = true;
            this.cbxBitrateMinPresets.Items.AddRange(new object[] {
            "Custom..."});
            this.cbxBitrateMinPresets.Location = new System.Drawing.Point(6, 116);
            this.cbxBitrateMinPresets.Name = "cbxBitrateMinPresets";
            this.cbxBitrateMinPresets.Size = new System.Drawing.Size(121, 21);
            this.cbxBitrateMinPresets.TabIndex = 21;
            this.cbxBitrateMinPresets.SelectedIndexChanged += new System.EventHandler(this.cbxBitrateMinPresets_SelectedIndexChanged);
            // 
            // radUseCbr
            // 
            this.radUseCbr.AutoSize = true;
            this.radUseCbr.Location = new System.Drawing.Point(6, 93);
            this.radUseCbr.Name = "radUseCbr";
            this.radUseCbr.Size = new System.Drawing.Size(101, 17);
            this.radUseCbr.TabIndex = 16;
            this.radUseCbr.TabStop = true;
            this.radUseCbr.Text = "Minimum bit rate";
            this.radUseCbr.UseVisualStyleBackColor = true;
            // 
            // label12
            // 
            this.label12.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(396, 74);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(43, 13);
            this.label12.TabIndex = 20;
            this.label12.Text = "Highest";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(34, 74);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(41, 13);
            this.label13.TabIndex = 19;
            this.label13.Text = "Lowest";
            // 
            // tbQuality
            // 
            this.tbQuality.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tbQuality.BackColor = System.Drawing.SystemColors.Window;
            this.tbQuality.Location = new System.Drawing.Point(37, 42);
            this.tbQuality.Maximum = 31;
            this.tbQuality.Minimum = 1;
            this.tbQuality.Name = "tbQuality";
            this.tbQuality.Size = new System.Drawing.Size(402, 45);
            this.tbQuality.TabIndex = 18;
            this.tbQuality.Value = 1;
            this.tbQuality.ValueChanged += new System.EventHandler(this.tbQuality_ValueChanged);
            // 
            // radUseQuality
            // 
            this.radUseQuality.AutoSize = true;
            this.radUseQuality.Checked = true;
            this.radUseQuality.Location = new System.Drawing.Point(6, 19);
            this.radUseQuality.Name = "radUseQuality";
            this.radUseQuality.Size = new System.Drawing.Size(125, 17);
            this.radUseQuality.TabIndex = 17;
            this.radUseQuality.TabStop = true;
            this.radUseQuality.Text = "Constant quality level";
            this.radUseQuality.UseVisualStyleBackColor = true;
            // 
            // chkFullRange
            // 
            this.chkFullRange.AutoSize = true;
            this.chkFullRange.Location = new System.Drawing.Point(120, 90);
            this.chkFullRange.Name = "chkFullRange";
            this.chkFullRange.Size = new System.Drawing.Size(65, 17);
            this.chkFullRange.TabIndex = 3;
            this.chkFullRange.Text = "Enabled";
            this.chkFullRange.UseVisualStyleBackColor = true;
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(3, 91);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(81, 13);
            this.label15.TabIndex = 34;
            this.label15.Text = "Full-range YUV:";
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.radUseQuality);
            this.groupBox1.Controls.Add(this.label13);
            this.groupBox1.Controls.Add(this.lblQualityLevel);
            this.groupBox1.Controls.Add(this.label12);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.radUseCbr);
            this.groupBox1.Controls.Add(this.txtBitrateBufferSize);
            this.groupBox1.Controls.Add(this.cbxBitrateMinPresets);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.lblBitrateMinSuffix);
            this.groupBox1.Controls.Add(this.txtBitrateMin);
            this.groupBox1.Controls.Add(this.tbQuality);
            this.groupBox1.Location = new System.Drawing.Point(6, 116);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(454, 151);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            // 
            // MPEG2VideoOptionsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.chkFullRange);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.txtBitrateMax);
            this.Controls.Add(this.lblBitrateMaxSuffix);
            this.Controls.Add(this.cbxBitrateMaxPresets);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.chkAutoRD);
            this.Controls.Add(this.cbxMotionEstimation);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.cbxBStrategy);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.cbxBFrameGroupSize);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.chkTwoPass);
            this.Controls.Add(this.txtSlices);
            this.Controls.Add(this.label1);
            this.Name = "MPEG1VideoOptionsControl";
            this.Size = new System.Drawing.Size(465, 315);
            ((System.ComponentModel.ISupportInitialize)(this.tbQuality)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtSlices;
        private System.Windows.Forms.CheckBox chkTwoPass;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbxBFrameGroupSize;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbxBStrategy;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cbxMotionEstimation;
        private System.Windows.Forms.CheckBox chkAutoRD;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label lblQualityLevel;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtBitrateBufferSize;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox txtBitrateMax;
        private System.Windows.Forms.Label lblBitrateMaxSuffix;
        private System.Windows.Forms.ComboBox cbxBitrateMaxPresets;
        private System.Windows.Forms.TextBox txtBitrateMin;
        private System.Windows.Forms.Label lblBitrateMinSuffix;
        private System.Windows.Forms.ComboBox cbxBitrateMinPresets;
        private System.Windows.Forms.RadioButton radUseCbr;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TrackBar tbQuality;
        private System.Windows.Forms.RadioButton radUseQuality;
        private System.Windows.Forms.CheckBox chkFullRange;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.GroupBox groupBox1;
    }
}
