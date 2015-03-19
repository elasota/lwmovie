using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;

namespace lwfe
{
    public partial class TaskMonitor : Form
    {
        private class ProcessIOConnector
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

        private class LogRelay
        {
            private TaskLogView _logView;

            public LogRelay(TaskLogView logView)
            {
                _logView = logView;
            }

            public void OnErrorLogMessage(object sender, System.Diagnostics.DataReceivedEventArgs e)
            {
                if (e.Data != null)
                {
                    _logView.Invoke((MethodInvoker)delegate
                    {
                        _logView.AppendText(e.Data);
                        _logView.AppendText(Environment.NewLine);
                    });
                }
            }
        }

        private Queue<ExecutionPlan> _planQueue = new Queue<ExecutionPlan>();
        private IEnumerable<ExecutionPlan> _executionPlans;

        public TaskMonitor(IEnumerable<ExecutionPlan> executionPlans)
        {
            InitializeComponent();

            _executionPlans = executionPlans;
        }

        private void ExecuteStages(ExecutionStage[] stages)
        {
            System.Diagnostics.Process[] stageProcesses = new System.Diagnostics.Process[stages.Length];
            List<TabPage> tabPages = new List<TabPage>();
            for (int i = 0; i < stages.Length; i++)
            {
                ExecutionStage stage = stages[i];

                LogRelay logRelay = null;
                this.Invoke((MethodInvoker)delegate
                {
                    string desc;
                    if (stages.Length == 1)
                        desc = "Task (Running)";
                    else if (i == 0)
                        desc = "Task (Running) ->";
                    else if (i == stages.Length - 1)
                        desc = "-> Task (Running)";
                    else
                        desc = "-> Task (Running) ->";

                    TabPage tabPage = new TabPage(desc);
                    tabPages.Add(tabPage);
                    tabLogFiles.TabPages.Add(tabPage);

                    TaskLogView logView = new TaskLogView();

                    tabPage.Controls.Add(logView);

                    logView.ProcessPath = (stage.ExePath);
                    {
                        string displayArgs = "";
                        foreach (string arg in stage.Args)
                        {
                            if (displayArgs != "")
                                displayArgs += " ";
                            displayArgs += "\"" + arg + "\"";
                        }
                        logView.Arguments = displayArgs;
                    }

                    logView.Size = tabPage.Size;
                    logView.Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top | AnchorStyles.Bottom;

                    logRelay = new LogRelay(logView);
                });

                ExecutionSet eSet = new ExecutionSet();
                System.Diagnostics.Process p = ExecutionSet.LaunchProcess(stage.ExePath, stage.Args, i != 0, i != stages.Length - 1, true);
                stageProcesses[i] = p;
                p.ErrorDataReceived += logRelay.OnErrorLogMessage;
                p.BeginErrorReadLine();

                if (i != 0)
                {
                    ProcessIOConnector connector = new ProcessIOConnector(stageProcesses[i - 1], p);
                    connector.RunThreaded();
                }
            }

            foreach (System.Diagnostics.Process p in stageProcesses)
            {
                p.WaitForExit();
            }

            this.Invoke((MethodInvoker)delegate
            {
                foreach (TabPage page in tabPages)
                    page.Text = page.Text.Replace("(Running)", "(Done)");
            });
        }

        private void ExecutePlan(ExecutionPlan plan)
        {
            List<ExecutionStage> stageList = new List<ExecutionStage>();
            foreach (ExecutionStage stage in plan.Stages)
                stageList.Add(stage);
            ExecuteStages(stageList.ToArray());

            this.Invoke((MethodInvoker)delegate
            {
                this.pbTaskProgressBar.Value++;
            });
            if (plan.CompletionCallback != null)
                plan.CompletionCallback();
        }

        private void ThreadRunExecutionPlans(object o)
        {
            HashSet<string> delayedCleanupFiles = new HashSet<string>();
            while(_planQueue.Count != 0)
            {
                ExecutionPlan plan = _planQueue.Dequeue();
                foreach (string filePath in plan.TemporaryFiles)
                    delayedCleanupFiles.Add(filePath);
                ExecutePlan(plan);
                foreach (string filePath in plan.CleanupFiles)
                {
                    // TODO: Catch delete failures
                    System.IO.File.Delete(filePath);
                    delayedCleanupFiles.Remove(filePath);
                }
            }

            foreach (string filePath in delayedCleanupFiles)
                System.IO.File.Delete(filePath);
        }

        private void RunExecutionPlans()
        {
            int numPlans = 0;
            foreach (ExecutionPlan plan in _executionPlans)
            {
                _planQueue.Enqueue(plan);
                numPlans++;
            }
            pbTaskProgressBar.Minimum = 0;
            pbTaskProgressBar.Value = 0;
            pbTaskProgressBar.Maximum = numPlans;

            Thread t = new Thread(this.ThreadRunExecutionPlans);
            t.Start(null);
        }

        private void TaskMonitor_Load(object sender, EventArgs e)
        {
            RunExecutionPlans();
        }
    }
}
