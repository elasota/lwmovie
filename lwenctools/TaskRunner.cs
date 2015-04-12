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

        public TaskRunner(ITaskRunnerMonitor monitor, IEnumerable<ExecutionPlan> executionPlans)
        {
            _monitor = monitor;
            _executionPlans = executionPlans;
        }

        private void ExecuteStages(ExecutionStage[] stages)
        {
            System.Diagnostics.Process[] stageProcesses = new System.Diagnostics.Process[stages.Length];

            IPlanMonitor planMonitor = _monitor.CreatePlanMonitor(stages.Length);

            for (int i = 0; i < stages.Length; i++)
            {
                ExecutionStage stage = stages[i];

                IStageMonitor stageMonitor = planMonitor.AddStage(stage.ExePath, stage.Args, i);

                ExecutionSet eSet = new ExecutionSet();
                System.Diagnostics.Process p = ExecutionSet.LaunchProcess(stage.ExePath, stage.Args, i != 0, i != stages.Length - 1, true);
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
                p.WaitForExit();
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

            Thread t = new Thread(this.ThreadRunExecutionPlans);
            t.Start(null);
        }
    }
}
