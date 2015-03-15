using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwfe
{
    public partial class FileSelector : UserControl
    {
        public enum EFileSelectionMode
        {
            Open,
            Save,
        };

        public FileSelector()
        {
            InitializeComponent();
        }

        public string DefaultDirectory { get; set; }

        public int DefaultFilterIndex { get; set; }

        [Description("Determines what type of file dialog is displayed when Browse... is clicked")]
        [Browsable(true)]
        public EFileSelectionMode FileSelectionMode { get; set; }

        [Description("File type filters displayed by the file dialog")]
        [Browsable(true)]
        public string FileTypeFilters { get; set; }

        [Browsable(true)]
        [EditorBrowsable(EditorBrowsableState.Always)]
        public event EventHandler FileChanged;

        public string FileName
        {
            get
            {
                return txtFilePath.Text;
            }
            set
            {
                string adjustedValue = (value == null) ? "" : value;

                if (adjustedValue != txtFilePath.Text)
                {
                    txtFilePath.Text = value;
                    if (this.FileChanged != null)
                        FileChanged(this, new EventArgs());
                }
            }
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            FileDialog fDialog;
            if (FileSelectionMode == EFileSelectionMode.Open)
                fDialog = new OpenFileDialog();
            else if (FileSelectionMode == EFileSelectionMode.Save)
                fDialog = new SaveFileDialog();
            else
                throw new InvalidOperationException();

            fDialog.InitialDirectory = DefaultDirectory;
            fDialog.Filter = FileTypeFilters;
            fDialog.FilterIndex = DefaultFilterIndex;
            fDialog.RestoreDirectory = false;

            if (fDialog.ShowDialog() == DialogResult.OK)
            {
                this.FileName = fDialog.FileName;
            }
        }
    }
}
