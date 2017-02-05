using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lwenctools
{
    public interface IStageMonitor
    {
        void OnErrorLogMessage(object sender, System.Diagnostics.DataReceivedEventArgs e);
    }

    public interface IPlanMonitor
    {
        IStageMonitor AddStage(string exePath, string[] args, int stageNum);
        void OnFinished();
    }

    public interface ITaskRunnerMonitor
    {
        void OnStarted(int numPlans);
        IPlanMonitor CreatePlanMonitor(int numStages);
        void OnFinished();
        void OnKillCleanup();
    }
}
