using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace lwfe
{
    public class ExecutionSet
    {
        private List<Process> _processes = new List<Process>();

        public IEnumerable<Process> Processes
        {
            get
            {
                return _processes;
            }
        }

        public static Process LaunchProcess(string exePath, string[] args, bool captureStdIn, bool captureStdOut, bool captureStdErr)
        {
            ProcessStartInfo psi = new ProcessStartInfo();
            string argsString = "";

            foreach(string arg in args)
            {
                if(argsString != "")
                    argsString += " ";
                argsString += "\"";
                string argEscaped = arg.Replace("\"", "\\\"");
                argsString += argEscaped;
                argsString += "\"";
            }

            psi.FileName = exePath;
            psi.Arguments = argsString;
            psi.UseShellExecute = false;
            psi.RedirectStandardInput = captureStdIn;
            psi.RedirectStandardOutput = captureStdOut;
            psi.RedirectStandardError = captureStdErr;
            psi.CreateNoWindow = true;

            return Process.Start(psi);
        }

        public void AddProcess(Process p)
        {
            _processes.Add(p);
        }

        public Process GetProcess(int index)
        {
            return _processes[index];
        }

        public ExecutionSet()
        {
        }
    }
}
