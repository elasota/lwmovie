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
using lwenctools;

namespace lwfe
{
    public partial class TaskMonitor : Form, ITaskRunnerMonitor
    {
        private TaskRunner _taskRunner;

        public TaskMonitor(IEnumerable<ExecutionPlan> executionPlans)
        {
            InitializeComponent();

            _taskRunner = new TaskRunner(this, executionPlans);
        }

        private void TaskMonitor_Load(object sender, EventArgs e)
        {
            _taskRunner.RunExecutionPlans();
        }

        void ITaskRunnerMonitor.OnStarted(int numPlans)
        {
            pbTaskProgressBar.Minimum = 0;
            pbTaskProgressBar.Value = 0;
            pbTaskProgressBar.Maximum = numPlans;
        }

        private class StageMonitor : IStageMonitor
        {
            private TaskLogView _logView;

            public StageMonitor(TaskLogView logView)
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

        private class PlanMonitor : IPlanMonitor
        {
            private List<TabPage> _tabPages = new List<TabPage>();
            private TaskMonitor _taskMonitor;
            private int _numStages;

            public PlanMonitor(TaskMonitor taskMonitor, int numStages)
            {
                _taskMonitor = taskMonitor;
                _numStages = numStages;
            }

            IStageMonitor IPlanMonitor.AddStage(string exePath, string[] args, int stageNum)
            {
                TaskLogView logView = null;
                _taskMonitor.Invoke((MethodInvoker)delegate
                {
                    string desc;
                    if (_numStages == 1)
                        desc = "Task (Running)";
                    else if (stageNum == 0)
                        desc = "Task (Running) ->";
                    else if (stageNum == _numStages - 1)
                        desc = "-> Task (Running)";
                    else
                        desc = "-> Task (Running) ->";

                    TabPage tabPage = new TabPage(desc);
                    _tabPages.Add(tabPage);
                    _taskMonitor.tabLogFiles.TabPages.Add(tabPage);

                    logView = new TaskLogView();

                    tabPage.Controls.Add(logView);

                    logView.ProcessPath = exePath;
                    {
                        string displayArgs = "";
                        foreach (string arg in args)
                        {
                            if (displayArgs != "")
                                displayArgs += " ";
                            displayArgs += "\"" + arg + "\"";
                        }
                        logView.Arguments = displayArgs;
                    }

                    logView.Size = tabPage.Size;
                    logView.Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top | AnchorStyles.Bottom;
                });

                return new StageMonitor(logView);
            }

            void IPlanMonitor.OnFinished()
            {
                _taskMonitor.Invoke((MethodInvoker)delegate
                {
                    foreach (TabPage page in _tabPages)
                        page.Text = page.Text.Replace("(Running)", "(Done)");

                    _taskMonitor.pbTaskProgressBar.Value++;
                });
            }
        }

        IPlanMonitor ITaskRunnerMonitor.CreatePlanMonitor(int numStages)
        {
            return new PlanMonitor(this, numStages);
        }

        void ITaskRunnerMonitor.OnFinished()
        {
        }
    }
}
