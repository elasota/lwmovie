namespace lwfe
{
    partial class TaskLogView
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
            this.label2 = new System.Windows.Forms.Label();
            this.txtProcessPath = new System.Windows.Forms.TextBox();
            this.txtArguments = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtConsoleOutput = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(0, 3);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(48, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Process:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(0, 32);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(60, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Arguments:";
            // 
            // txtProcessPath
            // 
            this.txtProcessPath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtProcessPath.Font = new System.Drawing.Font("Courier New", 8F);
            this.txtProcessPath.Location = new System.Drawing.Point(66, 0);
            this.txtProcessPath.Name = "txtProcessPath";
            this.txtProcessPath.ReadOnly = true;
            this.txtProcessPath.Size = new System.Drawing.Size(123, 20);
            this.txtProcessPath.TabIndex = 2;
            // 
            // txtArguments
            // 
            this.txtArguments.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtArguments.Font = new System.Drawing.Font("Courier New", 8F);
            this.txtArguments.Location = new System.Drawing.Point(66, 29);
            this.txtArguments.Name = "txtArguments";
            this.txtArguments.ReadOnly = true;
            this.txtArguments.Size = new System.Drawing.Size(123, 20);
            this.txtArguments.TabIndex = 3;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.txtConsoleOutput);
            this.groupBox1.Location = new System.Drawing.Point(3, 55);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(183, 85);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Console Output";
            // 
            // txtConsoleOutput
            // 
            this.txtConsoleOutput.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtConsoleOutput.Font = new System.Drawing.Font("Courier New", 8F);
            this.txtConsoleOutput.Location = new System.Drawing.Point(6, 19);
            this.txtConsoleOutput.Multiline = true;
            this.txtConsoleOutput.Name = "txtConsoleOutput";
            this.txtConsoleOutput.ReadOnly = true;
            this.txtConsoleOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtConsoleOutput.Size = new System.Drawing.Size(171, 60);
            this.txtConsoleOutput.TabIndex = 0;
            // 
            // TaskLogView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.txtArguments);
            this.Controls.Add(this.txtProcessPath);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "TaskLogView";
            this.Size = new System.Drawing.Size(189, 143);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtProcessPath;
        private System.Windows.Forms.TextBox txtArguments;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox txtConsoleOutput;
    }
}
