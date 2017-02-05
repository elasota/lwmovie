using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public class TheoraVideoSettings : IExecutionPlanSettings
    {
        public bool Use2Pass { get; set; }
        public int RateBufferSize { get; set; }
        public int BitratePresetIndex { get; set; }
        public int Bitrate { get; set; }
        public bool HaveKeyFrameRate { get; set; }
        public int KeyFrameRate { get; set; }
        public int Quality { get; set; }
        public bool UseQuality { get; set; }
        public bool UseCBR { get; set; }
        public bool SubsampleChroma { get; set; }
        public bool UseFullRangeYUV { get; set; }
        public int SpeedLevel { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
            int temp;
            if (int.TryParse(xml.GetAttribute("RateBufferSize"), out temp))
                RateBufferSize = temp;
            if (int.TryParse(xml.GetAttribute("BitratePresetIndex"), out temp))
                BitratePresetIndex = temp;
            if (int.TryParse(xml.GetAttribute("Bitrate"), out temp))
                Bitrate = temp;
            if (int.TryParse(xml.GetAttribute("KeyFrameRate"), out temp))
                KeyFrameRate = temp;
            if (int.TryParse(xml.GetAttribute("Quality"), out temp))
                Quality = temp;
            if (int.TryParse(xml.GetAttribute("SpeedLevel"), out temp))
                SpeedLevel = temp;

            Use2Pass = (xml.GetAttribute("Use2Pass") != "False");
            HaveKeyFrameRate = (xml.GetAttribute("HaveKeyFrameRate") != "False");
            UseQuality = (xml.GetAttribute("UseQuality") != "False");
            UseCBR = (xml.GetAttribute("UseCBR") != "False");
            SubsampleChroma = (xml.GetAttribute("SubsampleChroma") != "False");
            UseFullRangeYUV = (xml.GetAttribute("UseFullRangeYUV") != "False");
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
            xml.SetAttribute("RateBufferSize", RateBufferSize.ToString());
            xml.SetAttribute("BitratePresetIndex", BitratePresetIndex.ToString());
            xml.SetAttribute("Bitrate", Bitrate.ToString());
            xml.SetAttribute("KeyFrameRate", KeyFrameRate.ToString());
            xml.SetAttribute("Quality", Quality.ToString());
            xml.SetAttribute("Use2Pass", Use2Pass.ToString());
            xml.SetAttribute("HaveKeyFrameRate", HaveKeyFrameRate.ToString());
            xml.SetAttribute("UseCBR", UseCBR.ToString());
            xml.SetAttribute("UseQuality", UseQuality.ToString());
            xml.SetAttribute("SubsampleChroma", SubsampleChroma.ToString());
            xml.SetAttribute("UseFullRangeYUV", UseFullRangeYUV.ToString());
            xml.SetAttribute("SpeedLevel", SpeedLevel.ToString());
        }

        void IExecutionPlanSettings.CreateCommands(List<ExecutionPlan> plans, Dictionary<string, object> externalSettings, PlanCompletionDelegate pcd)
        {
            string lwmuxPath = (string)externalSettings["lwmuxPath"];
            string lwthencPath = (string)externalSettings["lwthencPath"];
            string ffmpegPath = (string)externalSettings["ffmpegPath"];
            string inputFile = (string)externalSettings["InputFile"];
            string outputFile = (string)externalSettings["OutputFile"];

            int numPasses = Use2Pass ? 2 : 1;

            for (int pass = 0; pass < numPasses; pass++)
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
                    if (SubsampleChroma)
                        stageArgs.Add("yuv420p12le");
                    else
                        stageArgs.Add("yuv444p12le");
                    stageArgs.Add("-an");
                    stageArgs.Add("-");

                    plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                }
                // Encoding stage
                {
                    List<string> stageArgs = new List<string>();
                    stageArgs.Add("-");

                    if (Use2Pass && pass == 0)
                    {
                        stageArgs.Add(outputFile + ".p");
                        stageArgs.Add("-pass1");
                    }
                    else
                        stageArgs.Add(outputFile);

                    if (Use2Pass && pass == 1)
                    {
                        stageArgs.Add("-pass2");
                        stageArgs.Add(outputFile + ".p");
                    }

                    if (UseQuality)
                    {
                        stageArgs.Add("-q");
                        stageArgs.Add(Quality.ToString());
                    }

                    if (UseCBR)
                    {
                        stageArgs.Add("-b");
                        stageArgs.Add((Bitrate * 1000).ToString());
                    }

                    stageArgs.Add("-speed");
                    stageArgs.Add(SpeedLevel.ToString());

                    stageArgs.Add("-buffer");
                    stageArgs.Add(RateBufferSize.ToString());

                    if (HaveKeyFrameRate)
                    {
                        stageArgs.Add("-k");
                        stageArgs.Add(KeyFrameRate.ToString());
                    }

                    if (UseFullRangeYUV)
                        stageArgs.Add("-full");

                    plan.AddStage(new ExecutionStage(lwthencPath, stageArgs.ToArray()));
                }

                if (!Use2Pass || pass == 1)
                    plan.CompletionCallback = pcd;

                if (Use2Pass && pass == 1)
                    plan.AddCleanupFile(outputFile + ".p");

                plans.Add(plan);
            }
        }

        bool IExecutionPlanSettings.Validate(List<string> outErrors)
        {
            bool isOK = true;
            if (HaveKeyFrameRate && KeyFrameRate < 1)
            {
                outErrors.Add("Key frame rate must be at least 1");
                isOK = false;
            }
            if ((UseQuality || !UseCBR) && Use2Pass)
            {
                outErrors.Add("2-pass encoding is only supported in constant bit rate mode");
                isOK = false;
            }
            return isOK;
        }

        public TheoraVideoSettings()
        {
            BitratePresetIndex = 4;
            SubsampleChroma = true;
            Quality = 32;
            RateBufferSize = 12;
            SpeedLevel = 1;
            UseQuality = true;
            KeyFrameRate = 256;
        }
    }
}
