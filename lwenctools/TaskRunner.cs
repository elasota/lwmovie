using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace lwenctools
{
    public class TaskRunner
    {
        private ITaskRunnerMonitor _monitor;
        private IEnumerable<ExecutionPlan> _executionPlans;
        private Queue<ExecutionPlan> _planQueue = new Queue<ExecutionPlan>();
        private bool _isKilled;
        private HashSet<System.Diagnostics.Process> _activeProcesses = new HashSet<System.Diagnostics.Process>();
        private Thread _thread;
        private AutoResetEvent _finishedEvent;
        private bool _isRunning;

        public bool IsRunning { get { return _isRunning; } }

        public TaskRunner(ITaskRunnerMonitor monitor, IEnumerable<ExecutionPlan> executionPlans)
        {
            _monitor = monitor;
            _executionPlans = executionPlans;
            _isKilled = false;
            _isRunning = true;
            _finishedEvent = new AutoResetEvent(false);
        }

        private void ExecuteStages(ExecutionStage[] stages)
        {
            System.Diagnostics.Process[] stageProcesses = new System.Diagnostics.Process[stages.Length];

            IPlanMonitor planMonitor = _monitor.CreatePlanMonitor(stages.Length);

            for (int i = 0; i < stages.Length; i++)
            {
                ExecutionStage stage = stages[i];

                IStageMonitor stageMonitor = planMonitor.AddStage(stage.ExePath, stage.Args, i);

                System.Diagnostics.Process p;
                lock (this)
                {
                    if (_isKilled)
                        break;
                    else
                    {
                        p = ExecutionSet.LaunchProcess(stage.ExePath, stage.Args, i != 0, i != stages.Length - 1, true);
                        _activeProcesses.Add(p);
                    }
                }
                stageProcesses[i] = p;
                p.ErrorDataReceived += stageMonitor.OnErrorLogMessage;
                p.BeginErrorReadLine();

                if (i != 0)
                {
                    ProcessIOConnector connector = new ProcessIOConnector(stageProcesses[i - 1], p);
                    connector.RunThreaded();
                }
            }

            foreach (System.Diagnostics.Process p in stageProcesses)
            {
                if (p != null)
                {
                    while (true)
                    {
                        bool exited = p.WaitForExit(1000);
                        if (exited)
                            break;
                    }
                    lock (this)
                    {
                        _activeProcesses.Remove(p);
                        p.Dispose();
                    }
                }
            }

            planMonitor.OnFinished();
        }

        private void ExecutePlan(ExecutionPlan plan, ITaskRunnerMonitor taskRunnerMonitor)
        {
            List<ExecutionStage> stageList = new List<ExecutionStage>();
            foreach (ExecutionStage stage in plan.Stages)
                stageList.Add(stage);
            ExecuteStages(stageList.ToArray());

            if (plan.CompletionCallback != null)
                plan.CompletionCallback();
        }

        private void ThreadRunExecutionPlans(object o)
        {
            HashSet<string> delayedCleanupFiles = new HashSet<string>();
            while (_planQueue.Count != 0)
            {
                ExecutionPlan plan = _planQueue.Dequeue();
                foreach (string filePath in plan.TemporaryFiles)
                    delayedCleanupFiles.Add(filePath);
                ExecutePlan(plan, _monitor);
                foreach (string filePath in plan.CleanupFiles)
                {
                    // TODO: Catch delete failures
                    System.IO.File.Delete(filePath);
                    delayedCleanupFiles.Remove(filePath);
                }
            }

            foreach (string filePath in delayedCleanupFiles)
                System.IO.File.Delete(filePath);

            _monitor.OnFinished();

            bool wasKilled;
            lock (this)
            {
                _isRunning = false;
                wasKilled = _isKilled;
            }
            
            if (wasKilled)
                _monitor.OnKillCleanup();
        }

        public void RunExecutionPlans()
        {
            int numPlans = 0;
            foreach (ExecutionPlan plan in _executionPlans)
            {
                _planQueue.Enqueue(plan);
                numPlans++;
            }

            _monitor.OnStarted(numPlans);

            _thread = new Thread(this.ThreadRunExecutionPlans);
            _thread.Start(null);
        }

        public void Kill()
        {
            if (_thread == null)
                return;

            lock (this)
            {
                if (_isKilled)
                    return;

                _isKilled = true;
                foreach (System.Diagnostics.Process p in _activeProcesses)
                {
                    try
                    {
                        p.Kill();
                    }
                    catch (System.InvalidOperationException)
                    {
                        // Handle process closing itself
                    }
                }
            }
        }
    }
}
