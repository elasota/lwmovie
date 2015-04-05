namespace lwenctools
{
    partial class CELTAudioOptionsControl
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
            this.chkVBR = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.cbxBitratePreset = new System.Windows.Forms.ComboBox();
            this.txtBitRate = new System.Windows.Forms.TextBox();
            this.lblBPS = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // chkVBR
            // 
            this.chkVBR.AutoSize = true;
            this.chkVBR.Location = new System.Drawing.Point(3, 3);
            this.chkVBR.Name = "chkVBR";
            this.chkVBR.Size = new System.Drawing.Size(99, 17);
            this.chkVBR.TabIndex = 0;
            this.chkVBR.Text = "Variable bit rate";
            this.chkVBR.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 29);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Target bit rate:";
            // 
            // cbxBitratePreset
            // 
            this.cbxBitratePreset.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitratePreset.FormattingEnabled = true;
            this.cbxBitratePreset.Items.AddRange(new object[] {
            "Custom..."});
            this.cbxBitratePreset.Location = new System.Drawing.Point(85, 26);
            this.cbxBitratePreset.Name = "cbxBitratePreset";
            this.cbxBitratePreset.Size = new System.Drawing.Size(121, 21);
            this.cbxBitratePreset.TabIndex = 2;
            this.cbxBitratePreset.SelectedIndexChanged += new System.EventHandler(this.cbxBitratePreset_SelectedIndexChanged);
            // 
            // txtBitRate
            // 
            this.txtBitRate.Location = new System.Drawing.Point(212, 27);
            this.txtBitRate.Name = "txtBitRate";
            this.txtBitRate.Size = new System.Drawing.Size(78, 20);
            this.txtBitRate.TabIndex = 3;
            // 
            // lblBPS
            // 
            this.lblBPS.AutoSize = true;
            this.lblBPS.Location = new System.Drawing.Point(296, 30);
            this.lblBPS.Name = "lblBPS";
            this.lblBPS.Size = new System.Drawing.Size(79, 13);
            this.lblBPS.TabIndex = 4;
            this.lblBPS.Text = "bits per second";
            // 
            // CELTAudioOptionsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lblBPS);
            this.Controls.Add(this.txtBitRate);
            this.Controls.Add(this.cbxBitratePreset);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.chkVBR);
            this.Name = "CELTAudioOptionsControl";
            this.Size = new System.Drawing.Size(382, 58);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox chkVBR;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbxBitratePreset;
        private System.Windows.Forms.TextBox txtBitRate;
        private System.Windows.Forms.Label lblBPS;
    }
}
