﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwfe
{
    public class CELTAudioSettings : IExecutionPlanSettings
    {
        public bool VBR { get; set; }
        public int BitRate { get; set; }
        public int BitRatePresetIndex { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
            xml.SetAttribute("VBR", VBR ? "True" : "False");
            xml.SetAttribute("BitRate", BitRate.ToString());
            xml.SetAttribute("BitRatePresetIndex", BitRatePresetIndex.ToString());
        }

        void IExecutionPlanSettings.CreateCommands(List<ExecutionPlan> plans, Dictionary<string, object> externalSettings, PlanCompletionDelegate pcd)
        {
            string lwmuxPath = (string)externalSettings["lwmuxPath"];
            string ffmpegPath = (string)externalSettings["ffmpegPath"];
            string inputFile = (string)externalSettings["InputFile"];
            string outputFile = (string)externalSettings["OutputFile"];

            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(ffmpegPath, new string[] { "-i", inputFile, "-acodec", "pcm_s16le", "-vn", "-y", outputFile + ".wav" });
                plan.AddStage(stage);
                plan.AddTemporaryFile(outputFile + ".wav");
                plans.Add(plan);
            }
            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage(lwmuxPath, new string[] { "importwav_celt", BitRate.ToString(), VBR ? "1" : "0", outputFile + ".wav", outputFile });
                plan.AddStage(stage);

                plan.CompletionCallback = pcd;
                plan.AddCleanupFile(outputFile + ".wav");
                plans.Add(plan);
            }
        }

        public CELTAudioSettings()
        {
            BitRate = 192000;
            BitRatePresetIndex = 6;
            VBR = true;
        }
    }
}
