using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using lwenctools;

namespace lwenccmd
{
    class Program
    {
        class StageMonitor : IStageMonitor
        {
            private object _consoleMutex;
            private string _linePrefix;

            void IStageMonitor.OnErrorLogMessage(object sender, System.Diagnostics.DataReceivedEventArgs e)
            {
                if (e.Data != null)
                {
                    lock (_consoleMutex)
                    {
                        Console.Write(_linePrefix);
                        Console.WriteLine(e.Data);
                    }
                }
            }

            public StageMonitor(string linePrefix, object consoleMutex)
            {
                _linePrefix = linePrefix;
                _consoleMutex = consoleMutex;
            }
        }

        class PlanMonitor : IPlanMonitor
        {
            private object _consoleMutex;

            IStageMonitor IPlanMonitor.AddStage(string exePath, string[] args, int stageNum)
            {
                lock (_consoleMutex)
                {
                    Console.Write("Stage " + stageNum.ToString() + ": \"" + exePath + "\"");
                    foreach (string arg in args)
                        Console.Write(" \"" + arg + "\"");
                    Console.Write(Environment.NewLine);
                }
                return new StageMonitor(stageNum.ToString() + ": ", _consoleMutex);
            }

            void IPlanMonitor.OnFinished()
            {
                lock (_consoleMutex)
                {
                    Console.WriteLine("Plan finished");
                }
            }

            public PlanMonitor(object consoleMutex)
            {
                _consoleMutex = consoleMutex;
            }
        }

        class TaskRunnerMonitor : ITaskRunnerMonitor
        {
            private object _consoleMutex;
            System.Threading.EventWaitHandle _waitHandle;

            public TaskRunnerMonitor(object consoleMutex, System.Threading.EventWaitHandle waitHandle)
            {
                _consoleMutex = consoleMutex;
                _waitHandle = waitHandle;
            }

            void ITaskRunnerMonitor.OnStarted(int numPlans)
            {
                lock (_consoleMutex)
                {
                    Console.WriteLine("Starting tasks (" + numPlans.ToString() + " plans)");
                }
            }

            IPlanMonitor ITaskRunnerMonitor.CreatePlanMonitor(int numStages)
            {
                return new PlanMonitor(_consoleMutex);
            }

            void ITaskRunnerMonitor.OnFinished()
            {
                lock (_consoleMutex)
                {
                    Console.WriteLine("All plans finished");
                }

                _waitHandle.Set();
            }
        };

        static void ShowUsage()
        {
            Console.Error.WriteLine("Usage: lwenccmd [options] <project file.lwproj>");
            Console.Error.WriteLine("Options:");
            Console.Error.WriteLine("   -reencode: Re-encodes all streams from input.");
        }

        static int Main(string[] args)
        {
            int currentArg = 0;
            bool reencodeAll = false;
            while (true)
            {
                if (currentArg == args.Length)
                {
                    ShowUsage();
                    return -1;
                }
                string option = args[currentArg];
                if (option.Length == 0)
                {
                    ShowUsage();
                    return -1;
                }
                if (option == "-reencode")
                    reencodeAll = true;
                else
                    break;
                currentArg++;
            }

            if (currentArg != args.Length - 1)
            {
                ShowUsage();
                return -1;
            }

            string projFileName = args[currentArg];

            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(projFileName);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("A problem occurred while loading the project file:");
                Console.Error.WriteLine(ex.Message);
                return -1;
            }

            lwenctools.EncodeSettingsProject proj = new lwenctools.EncodeSettingsProject();
            proj.LoadFromXml(doc);

            if (reencodeAll)
            {
                proj.VideoUseIntermediate = false;
                foreach (EncodeSettingsProject.AudioStreamSettings audioStream in proj.AudioStreams)
                    audioStream.UseIntermediate = false;
            }

            List<string> errors = new List<string>();
            if (!proj.Validate(errors))
            {
                Console.Error.WriteLine("Errors occurred while validating the encode project:");
                foreach (string error in errors)
                    Console.Error.WriteLine(errors);
                return -1;
            }

            Dictionary<string, object> settings = new Dictionary<string, object>();

            proj.CreateDefaultSettings(settings);
            List<lwenctools.ExecutionPlan> ePlans = proj.CreateExecutionPlans(settings);

            object consoleMutex = new object();
            System.Threading.EventWaitHandle finishEvent = new System.Threading.AutoResetEvent(false);
            lwenctools.TaskRunner taskRunner = new lwenctools.TaskRunner(new TaskRunnerMonitor(consoleMutex, finishEvent), ePlans);
            taskRunner.RunExecutionPlans();

            finishEvent.WaitOne();

            return 0;
        }
    }
}
