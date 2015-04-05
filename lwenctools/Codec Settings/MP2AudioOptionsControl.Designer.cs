namespace lwenctools
{
    partial class MP2AudioOptionsControl
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
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cbxBitratePreset
            // 
            this.cbxBitratePreset.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxBitratePreset.FormattingEnabled = true;
            this.cbxBitratePreset.Location = new System.Drawing.Point(82, 3);
            this.cbxBitratePreset.Name = "cbxBitratePreset";
            this.cbxBitratePreset.Size = new System.Drawing.Size(121, 21);
            this.cbxBitratePreset.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(0, 6);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Target bit rate:";
            // 
            // MP2AudioOptionsControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.cbxBitratePreset);
            this.Controls.Add(this.label1);
            this.Name = "MP2AudioOptionsControl";
            this.Size = new System.Drawing.Size(216, 32);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbxBitratePreset;
        private System.Windows.Forms.Label label1;
    }
}
