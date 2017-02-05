namespace lwfe
{
    partial class TaskMonitor
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
            this.tabLogFiles = new System.Windows.Forms.TabControl();
            this.pbTaskProgressBar = new System.Windows.Forms.ProgressBar();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // tabLogFiles
            // 
            this.tabLogFiles.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabLogFiles.Location = new System.Drawing.Point(12, 28);
            this.tabLogFiles.Name = "tabLogFiles";
            this.tabLogFiles.SelectedIndex = 0;
            this.tabLogFiles.Size = new System.Drawing.Size(521, 265);
            this.tabLogFiles.TabIndex = 0;
            // 
            // pbTaskProgressBar
            // 
            this.pbTaskProgressBar.Location = new System.Drawing.Point(127, 9);
            this.pbTaskProgressBar.Name = "pbTaskProgressBar";
            this.pbTaskProgressBar.Size = new System.Drawing.Size(406, 13);
            this.pbTaskProgressBar.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(109, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Overall task progress:";
            // 
            // TaskMonitor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(545, 305);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.pbTaskProgressBar);
            this.Controls.Add(this.tabLogFiles);
            this.Name = "TaskMonitor";
            this.Text = "lwmovie Task Monitor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.TaskMonitor_FormClosing);
            this.Load += new System.EventHandler(this.TaskMonitor_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabLogFiles;
        private System.Windows.Forms.ProgressBar pbTaskProgressBar;
        private System.Windows.Forms.Label label1;
    }
}