using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lwenctools
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
        public IEnumerable<string> TemporaryFiles
        {
            get
            {
                return _temporaryFiles;
            }
        }
        public IEnumerable<string> CleanupFiles
        {
            get
            {
                return _cleanupFiles;
            }
        }

        public PlanCompletionDelegate CompletionCallback { get; set; }

        private List<ExecutionStage> _stages = new List<ExecutionStage>();
        private List<string> _temporaryFiles = new List<string>();
        private List<string> _cleanupFiles = new List<string>();

        public void AddStage(ExecutionStage stage)
        {
            _stages.Add(stage);
        }

        public void AddTemporaryFile(string path)
        {
            _temporaryFiles.Add(path);
        }

        public void AddCleanupFile(string path)
        {
            _cleanupFiles.Add(path);
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
