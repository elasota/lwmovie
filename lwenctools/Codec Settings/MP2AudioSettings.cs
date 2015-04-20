using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public class MP2AudioSettings : IExecutionPlanSettings
    {
        public int BitRate { get; set; }
        public int BitRatePresetIndex { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
            int temp;
            if (int.TryParse(xml.GetAttribute("BitRate"), out temp))
                BitRate = temp;
            if (int.TryParse(xml.GetAttribute("BitRatePresetIndex"), out temp))
                BitRatePresetIndex = temp;
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
            xml.SetAttribute("BitRate", BitRate.ToString());
            xml.SetAttribute("BitRatePresetIndex", BitRatePresetIndex.ToString());
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
                ExecutionStage stage = new ExecutionStage(ffmpegPath, new string[] { "-i", inputFile, "-acodec", "libtwolame", "-vn", "-b:a", BitRate.ToString(), "-y", outputFile + ".mp2" });
                plan.AddStage(stage);
                plan.AddTemporaryFile(outputFile + ".mp2");
                plan.CompletionCallback = pcd;
                plans.Add(plan);
            }
            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(lwmuxPath, new string[] { "importmp2", outputFile + ".mp2", outputFile, metaID });
                plan.AddStage(stage);

                plan.CompletionCallback = pcd;
                plan.AddCleanupFile(outputFile + ".mp2");
                plans.Add(plan);
            }
        }

        bool IExecutionPlanSettings.Validate(List<string> outErrors)
        {
            return true;
        }

        public MP2AudioSettings()
        {
            BitRate = 192000;
            BitRatePresetIndex = 9;
        }
    }
}
