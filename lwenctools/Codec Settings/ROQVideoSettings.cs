using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public class ROQVideoSettings : IExecutionPlanSettings
    {
        public int KeyFrameRate { get; set; }
        public int ThresholdPower { get; set; }
        public int NumCBPhases { get; set; }

        public int BitratePresetIndex { get; set; }
        public int Bitrate { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
            int temp;
            if (int.TryParse(xml.GetAttribute("KeyFrameRate"), out temp))
                KeyFrameRate = temp;
            if (int.TryParse(xml.GetAttribute("ThresholdPower"), out temp))
                ThresholdPower = temp;
            if (int.TryParse(xml.GetAttribute("NumCBPhases"), out temp))
                NumCBPhases = temp;
            
            if (int.TryParse(xml.GetAttribute("BitratePresetIndex"), out temp))
                BitratePresetIndex = temp;
            if (int.TryParse(xml.GetAttribute("Bitrate"), out temp))
                Bitrate = temp;
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
            xml.SetAttribute("KeyFrameRate", KeyFrameRate.ToString());
            xml.SetAttribute("ThresholdPower", ThresholdPower.ToString());
            xml.SetAttribute("NumCBPhases", NumCBPhases.ToString());
            
            xml.SetAttribute("BitratePresetIndex", BitratePresetIndex.ToString());
            xml.SetAttribute("Bitrate", Bitrate.ToString());
        }

        void IExecutionPlanSettings.CreateCommands(List<ExecutionPlan> plans, Dictionary<string, object> externalSettings, PlanCompletionDelegate pcd)
        {
            string lwmuxPath = (string)externalSettings["lwmuxPath"];
            string lwroqencPath = (string)externalSettings["lwroqencPath"];
            string ffmpegPath = (string)externalSettings["ffmpegPath"];
            string inputFile = (string)externalSettings["InputFile"];
            string outputFile = (string)externalSettings["OutputFile"];

            {
                ExecutionPlan plan = new ExecutionPlan();

                // Loading stage
                {
                    List<string> stageArgs = new List<string>();
                    stageArgs.Add("-i");
                    stageArgs.Add(inputFile);
                    stageArgs.Add("-f");
                    stageArgs.Add("yuv4mpegpipe");
                    stageArgs.Add("-strict");
                    stageArgs.Add("-1");
                    stageArgs.Add("-pix_fmt");
                    stageArgs.Add("yuv444p12le");
                    stageArgs.Add("-an");
                    stageArgs.Add("-");

                    plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                }
                // Encoding stage
                {
                    int keyFrameMultiplier = 4;
                    int keyRateDivision = KeyFrameRate - 1 + keyFrameMultiplier;

                    double bitRateAdjusted = ((double)Bitrate) * (double)KeyFrameRate / ((double)keyRateDivision);
                    int iBitrate = (int)(bitRateAdjusted * (double)keyFrameMultiplier);
                    int pBitrate = (int)bitRateAdjusted;

                    List<string> stageArgs = new List<string>();
                    stageArgs.Add("-");
                    stageArgs.Add(outputFile);
                    stageArgs.Add(iBitrate.ToString());
                    stageArgs.Add(pBitrate.ToString());
                    stageArgs.Add(KeyFrameRate.ToString());
                    stageArgs.Add((1 << ThresholdPower).ToString());
                    stageArgs.Add(NumCBPhases.ToString());
                    stageArgs.Add("lwiv");

                    plan.AddStage(new ExecutionStage(lwroqencPath, stageArgs.ToArray()));
                }
                plan.CompletionCallback = pcd;
                plans.Add(plan);
            }
        }

        bool IExecutionPlanSettings.Validate(List<string> outErrors)
        {
            bool isOK = true;
            if (KeyFrameRate < 1)
            {
                outErrors.Add("Key frame rate must be at least 1");
                isOK = false;
            }
            return isOK;
        }

        public ROQVideoSettings()
        {
            KeyFrameRate = 999999;
            ThresholdPower = 10;
            NumCBPhases = 10;

            BitratePresetIndex = 4;
        }
    }
}
