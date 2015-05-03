namespace lwenctools
{
    partial class ROQVideoOptionsControl
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
            this.cbxBitratePreset = new System.Windows.Forms.ComboBox();
            this.txtBitrate = new System.Windows.Forms.TextBox();
            this.lblBitrateSuffix = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.txtKeyFrameRate = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.tbCBThreshold = new System.Windows.Forms.TrackBar();
            this.label4 = new System.Windows.Forms.Label();
            this.tbNumCBPhases = new System.Windows.Forms.TrackBar();
            ((System.ComponentModel.ISupportInitialize)(this.tbCBThreshold)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbNumCBPhases)).BeginInit();
            this.SuspendLayout();
            // 
            // cbxBitratePreset
            // 
            this.cbxBitratePreset.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitratePreset.FormattingEnabled = true;
            this.cbxBitratePreset.Items.AddRange(new object[] {
            "Custom..."});
            this.cbxBitratePreset.Location = new System.Drawing.Point(3, 3);
            this.cbxBitratePreset.Name = "cbxBitratePreset";
            this.cbxBitratePreset.Size = new System.Drawing.Size(121, 21);
            this.cbxBitratePreset.TabIndex = 0;
            this.cbxBitratePreset.SelectedIndexChanged += new System.EventHandler(this.cbxBitratePreset_SelectedIndexChanged);
            // 
            // txtBitrate
            // 
            this.txtBitrate.Location = new System.Drawing.Point(130, 4);
            this.txtBitrate.Name = "txtBitrate";
            this.txtBitrate.Size = new System.Drawing.Size(103, 20);
            this.txtBitrate.TabIndex = 1;
            // 
            // lblBitrateSuffix
            // 
            this.lblBitrateSuffix.AutoSize = true;
            this.lblBitrateSuffix.Location = new System.Drawing.Point(239, 7);
            this.lblBitrateSuffix.Name = "lblBitrateSuffix";
            this.lblBitrateSuffix.Size = new System.Drawing.Size(24, 13);
            this.lblBitrateSuffix.TabIndex = 2;
            this.lblBitrateSuffix.Text = "bps";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 33);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(83, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Key frame every";
            // 
            // txtKeyFrameRate
            // 
            this.txtKeyFrameRate.Location = new System.Drawing.Point(92, 30);
            this.txtKeyFrameRate.Name = "txtKeyFrameRate";
            this.txtKeyFrameRate.Size = new System.Drawing.Size(100, 20);
            this.txtKeyFrameRate.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(198, 33);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(38, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "frames";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 58);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(141, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Codebook Threshold Range";
            // 
            // tbCBThreshold
            // 
            this.tbCBThreshold.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tbCBThreshold.BackColor = System.Drawing.SystemColors.Window;
            this.tbCBThreshold.Location = new System.Drawing.Point(6, 74);
            this.tbCBThreshold.Maximum = 12;
            this.tbCBThreshold.Minimum = 5;
            this.tbCBThreshold.Name = "tbCBThreshold";
            this.tbCBThreshold.Size = new System.Drawing.Size(269, 45);
            this.tbCBThreshold.TabIndex = 7;
            this.tbCBThreshold.Value = 5;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 122);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(184, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "Codebook Refinement Thoroughness";
            // 
            // tbNumCBPhases
            // 
            this.tbNumCBPhases.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tbNumCBPhases.BackColor = System.Drawing.SystemColors.Window;
            this.tbNumCBPhases.Location = new System.Drawing.Point(6, 138);
            this.tbNumCBPhases.Maximum = 12;
            this.tbNumCBPhases.Name = "tbNumCBPhases";
            this.tbNumCBPhases.Size = new System.Drawing.Size(269, 45);
            this.tbNumCBPhases.TabIndex = 9;
            // 
            // ROQVideoOptionsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tbNumCBPhases);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.tbCBThreshold);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtKeyFrameRate);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblBitrateSuffix);
            this.Controls.Add(this.txtBitrate);
            this.Controls.Add(this.cbxBitratePreset);
            this.Name = "ROQVideoOptionsControl";
            this.Size = new System.Drawing.Size(278, 193);
            ((System.ComponentModel.ISupportInitialize)(this.tbCBThreshold)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbNumCBPhases)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbxBitratePreset;
        private System.Windows.Forms.TextBox txtBitrate;
        private System.Windows.Forms.Label lblBitrateSuffix;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtKeyFrameRate;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TrackBar tbCBThreshold;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TrackBar tbNumCBPhases;
    }
}
