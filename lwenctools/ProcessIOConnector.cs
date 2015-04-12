using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Threading;

namespace lwenctools
{
    public class ProcessIOConnector
    {
        private Process _sourceProcess;
        private Process _destProcess;

        private byte[] _buffer = new byte[32768];

        public ProcessIOConnector(Process sourceProcess, Process destProcess)
        {
            _sourceProcess = sourceProcess;
            _destProcess = destProcess;
        }

        private void ThreadConnectIO(object o)
        {
            while (true)
            {
                int nRead = _sourceProcess.StandardOutput.BaseStream.Read(_buffer, 0, _buffer.Length);
                if (nRead == 0)
                    break;
                try
                {
                    _destProcess.StandardInput.BaseStream.Write(_buffer, 0, nRead);
                }
                catch (System.IO.IOException)
                {
                    // TODO?
                    break;
                }
            }
            _destProcess.StandardInput.Close();
        }

        public void RunThreaded()
        {
            Thread t = new Thread(this.ThreadConnectIO);
            t.Start(null);
        }
    }
}
