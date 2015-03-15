using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lwfe
{
    public class ExecutionPlan
    {
        public IEnumerable<ExecutionStage> Stages
        {
            get
            {
                return _stages;
            }
        }

        public PlanCompletionDelegate CompletionCallback { get; set; }

        private List<ExecutionStage> _stages = new List<ExecutionStage>();

        public void AddStage(ExecutionStage stage)
        {
            _stages.Add(stage);
        }
    }

    public class ExecutionStage
    {
        public string ExePath { get; private set; }
        public string[] Args { get; private set; }

        public ExecutionStage(string exePath, string[] args)
        {
            ExePath = exePath;
            Args = args;
        }
    }
}
