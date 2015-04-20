using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    class ADPCMAudioSettings : IExecutionPlanSettings
    {
        public bool VBR { get; set; }
        public int BitRate { get; set; }
        public int BitRatePresetIndex { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
        }

        void IExecutionPlanSettings.CreateCommands(List<ExecutionPlan> plans, Dictionary<string, object> externalSettings, PlanCompletionDelegate pcd)
        {
            string lwmuxPath = (string)externalSettings["lwmuxPath"];
            string ffmpegPath = (string)externalSettings["ffmpegPath"];
            string inputFile = (string)externalSettings["InputFile"];
            string outputFile = (string)externalSettings["OutputFile"];
            string metaID = (string)externalSettings["MetaID"];

            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(ffmpegPath, new string[] { "-i", inputFile, "-acodec", "pcm_s16le", "-vn", "-y", outputFile + ".wav" });
                plan.AddStage(stage);
                plan.AddTemporaryFile(outputFile + ".wav");
                plans.Add(plan);
            }
            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(lwmuxPath, new string[] { "importwav_adpcm", outputFile + ".wav", outputFile, metaID });
                plan.AddStage(stage);

                plan.CompletionCallback = pcd;
                plan.AddCleanupFile(outputFile + ".wav");
                plans.Add(plan);
            }
        }

        bool IExecutionPlanSettings.Validate(List<string> outErrors)
        {
            return true;
        }

        public ADPCMAudioSettings()
        {
        }
    }
}
