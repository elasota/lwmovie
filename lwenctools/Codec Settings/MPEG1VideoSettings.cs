using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public class MPEG1VideoSettings : IExecutionPlanSettings
    {
        public int MaxSlices { get; set; }
        public bool TwoPass { get; set; }
        public bool AutoRD { get; set; }
        public bool FullRange { get; set; }
        public int NumBFrames { get; set; }
        public int BStrategy { get; set; }
        public bool UseCBR { get; set; }
        public bool UseQuality { get; set; }
        public int Quality { get; set; }
        public int BitrateMinPresetIndex { get; set; }
        public int BitrateMin { get; set; }
        public int BitrateMaxPresetIndex { get; set; }
        public int BitrateMax { get; set; }
        public int BitrateBufferSize { get; set; }
        public int MotionEstimationPresetIndex { get; set; }

        void IExecutionPlanSettings.LoadFromXml(XmlElement xml)
        {
            int temp;
            if (int.TryParse(xml.GetAttribute("MaxSlices"), out temp))
                MaxSlices = temp;
            TwoPass = (xml.GetAttribute("TwoPass") != "False");
            AutoRD = (xml.GetAttribute("AutoRD") != "False");
            FullRange = (xml.GetAttribute("FullRange") != "False");
            if (int.TryParse(xml.GetAttribute("BFrames"), out temp))
                NumBFrames = temp;
            if (int.TryParse(xml.GetAttribute("BStrategy"), out temp))
                BStrategy = temp;
            UseCBR = (xml.GetAttribute("UseCBR") != "False");
            UseQuality = (xml.GetAttribute("UseQuality") != "False");
            if (int.TryParse(xml.GetAttribute("Quality"), out temp))
                Quality = temp;
            if (int.TryParse(xml.GetAttribute("BitrateMinPreset"), out temp))
                BitrateMinPresetIndex = temp;
            if (int.TryParse(xml.GetAttribute("BitrateMin"), out temp))
                BitrateMin = temp;
            if (int.TryParse(xml.GetAttribute("BitrateBufferSize"), out temp))
                BitrateBufferSize = temp;
            if (int.TryParse(xml.GetAttribute("BitrateMaxPreset"), out temp))
                BitrateMaxPresetIndex = temp;
            if (int.TryParse(xml.GetAttribute("BitrateMax"), out temp))
                BitrateMax = temp;
            if (int.TryParse(xml.GetAttribute("MotionEstimationPresetIndex"), out temp))
                MotionEstimationPresetIndex = temp;
        }

        void IExecutionPlanSettings.SaveToXml(XmlElement xml)
        {
            xml.SetAttribute("MaxSlices", MaxSlices.ToString());
            xml.SetAttribute("TwoPass", TwoPass ? "True" : "False");
            xml.SetAttribute("AutoRD", AutoRD ? "True" : "False");
            xml.SetAttribute("FullRange", FullRange ? "True" : "False");
            xml.SetAttribute("BFrames", NumBFrames.ToString());
            xml.SetAttribute("BStrategy", BStrategy.ToString());
            xml.SetAttribute("UseCBR", UseCBR ? "True" : "False");
            xml.SetAttribute("UseQuality", UseQuality ? "True" : "False");
            xml.SetAttribute("Quality", Quality.ToString());
            xml.SetAttribute("BitrateMinPreset", BitrateMinPresetIndex.ToString());
            xml.SetAttribute("BitrateMin", BitrateMin.ToString());
            xml.SetAttribute("BitrateBufferSize", BitrateBufferSize.ToString());
            xml.SetAttribute("BitrateMaxPreset", BitrateMaxPresetIndex.ToString());
            xml.SetAttribute("BitrateMax", BitrateMax.ToString());
            xml.SetAttribute("MotionEstimationPresetIndex", MotionEstimationPresetIndex.ToString());
        }

        void IExecutionPlanSettings.CreateCommands(List<ExecutionPlan> plans, Dictionary<string, object> externalSettings, PlanCompletionDelegate pcd)
        {
            string lwmuxPath = (string)externalSettings["lwmuxPath"];
            string rerangePath = (string)externalSettings["lwrerangePath"];
            string ffmpegPath = (string)externalSettings["ffmpegPath"];
            string inputFile = (string)externalSettings["InputFile"];
            string outputFile = (string)externalSettings["OutputFile"];

            List<string> ffEncArgs = new List<string>();
            List<string> ffPass1Args = new List<string>();
            List<string> ffPass2Args = new List<string>();
            ffEncArgs.Add("-idct");
            ffEncArgs.Add("int");

            if (MaxSlices > 0)
            {
                ffEncArgs.Add("-slices");
                ffEncArgs.Add(MaxSlices.ToString());
            }

            if (AutoRD)
            {
                ffEncArgs.Add("-trellis");
                ffEncArgs.Add("2");
                ffEncArgs.Add("-mbd");
                ffEncArgs.Add("rd");
                ffEncArgs.Add("-cmp");
                ffEncArgs.Add("2");
                ffEncArgs.Add("-subcmp");
                ffEncArgs.Add("2");
                ffEncArgs.Add("-skipcmp");
                ffEncArgs.Add("2");
                ffEncArgs.Add("-mbcmp");
                ffEncArgs.Add("2");
                ffEncArgs.Add("-mpv_flags");
                ffEncArgs.Add("+qp_rd+mv0");
            }

            ffEncArgs.Add("-bf");
            ffEncArgs.Add(NumBFrames.ToString());
            ffPass1Args.Add("-b_strategy");
            ffPass1Args.Add(BStrategy.ToString());

            if (UseCBR)
            {
                // TODO
            }

            if (UseQuality)
            {
                ffEncArgs.Add("-q:v");
                ffEncArgs.Add((32 - Quality).ToString());

                // Stupid trick to avoid problems where ffmpeg rejects the video because the bitrate is too low, even though
                // it'll make no attempt to actually hit the rate.
                ffEncArgs.Add("-b:v");
                ffEncArgs.Add("15000k");
            }

            ffEncArgs.Add("-threads");
            ffEncArgs.Add(Environment.ProcessorCount.ToString());

            ffEncArgs.Add("-an");
            ffEncArgs.Add("-y");

            ffEncArgs.Add("-maxrate");
            ffEncArgs.Add(BitrateMax.ToString() + "k");

            if (TwoPass)
            {
                // Pass 1
                {
                    ExecutionPlan plan = new ExecutionPlan();

                    if (FullRange)
                    {
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
                            stageArgs.Add("yuv420p12le");
                            stageArgs.Add("-");

                            plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                        }
                        // Rerange stage
                        {
                            plan.AddStage(new ExecutionStage(rerangePath, new string[0]));
                        }
                        // Encoding stage
                        {
                            List<string> stageArgs = new List<string>();
                            stageArgs.Add("-i");
                            stageArgs.Add("-");
                            stageArgs.AddRange(ffEncArgs);
                            stageArgs.AddRange(ffPass1Args);
                            stageArgs.Add("-f");
                            stageArgs.Add("mpeg1video");
                            stageArgs.Add("-pass");
                            stageArgs.Add("1");
                            stageArgs.Add("-passlogfile");
                            stageArgs.Add(outputFile + ".p");
                            stageArgs.Add("NUL");

                            plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                        }
                    }
                    else
                    {
                        List<string> stageArgs = new List<string>();
                        stageArgs.Add("-i");
                        stageArgs.Add(inputFile);
                        stageArgs.AddRange(ffEncArgs);
                        stageArgs.AddRange(ffPass1Args);
                        stageArgs.Add("-f");
                        stageArgs.Add("mpeg1video");
                        stageArgs.Add("-pass");
                        stageArgs.Add("1");
                        stageArgs.Add("-passlogfile");
                        stageArgs.Add(outputFile + ".p");
                        stageArgs.Add("NUL");

                        plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                    }
                    plan.AddTemporaryFile(outputFile + ".p-0.log");
                    plans.Add(plan);
                }
                // Pass 2
                {
                    ExecutionPlan plan = new ExecutionPlan();

                    if (FullRange)
                    {
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
                            stageArgs.Add("yuv420p12le");
                            stageArgs.Add("-");

                            plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                        }
                        // Rerange stage
                        {
                            plan.AddStage(new ExecutionStage(rerangePath, new string[0]));
                        }
                        // Encoding stage
                        {
                            List<string> stageArgs = new List<string>();
                            stageArgs.Add("-i");
                            stageArgs.Add("-");
                            stageArgs.AddRange(ffEncArgs);
                            stageArgs.AddRange(ffPass2Args);
                            stageArgs.Add("-f");
                            stageArgs.Add("mpeg1video");
                            stageArgs.Add("-pass");
                            stageArgs.Add("2");
                            stageArgs.Add("-passlogfile");
                            stageArgs.Add(outputFile + ".p");
                            stageArgs.Add(outputFile + ".m1v");

                            plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                        }
                    }
                    else
                    {
                        List<string> stageArgs = new List<string>();
                        stageArgs.Add("-i");
                        stageArgs.Add(inputFile);
                        stageArgs.AddRange(ffEncArgs);
                        stageArgs.AddRange(ffPass2Args);
                        stageArgs.Add("-f");
                        stageArgs.Add("mpeg1video");
                        stageArgs.Add("-pass");
                        stageArgs.Add("2");
                        stageArgs.Add("-passlogfile");
                        stageArgs.Add(outputFile + ".p");
                        stageArgs.Add(outputFile + ".m1v");

                        plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                    }
                    plan.AddCleanupFile(outputFile + ".p-0.log");
                    plan.AddTemporaryFile(outputFile + ".m1v");
                    plans.Add(plan);
                }
            }
            else
            {
                // 1-pass mode
                ExecutionPlan plan = new ExecutionPlan();

                if (FullRange)
                {
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
                        stageArgs.Add("yuv420p12le");
                        stageArgs.Add("-");

                        plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                    }
                    // Rerange stage
                    {
                        plan.AddStage(new ExecutionStage(rerangePath, new string[0]));
                    }
                    // Encoding stage
                    {
                        List<string> stageArgs = new List<string>();
                        stageArgs.Add("-i");
                        stageArgs.Add("-");
                        stageArgs.AddRange(ffEncArgs);
                        stageArgs.AddRange(ffPass1Args);
                        stageArgs.AddRange(ffPass2Args);
                        stageArgs.Add("-f");
                        stageArgs.Add("mpeg1video");
                        stageArgs.Add(outputFile + ".m1v");

                        plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                    }
                }
                else
                {
                    List<string> stageArgs = new List<string>();
                    stageArgs.Add("-i");
                    stageArgs.Add(inputFile);
                    stageArgs.AddRange(ffEncArgs);
                    stageArgs.AddRange(ffPass1Args);
                    stageArgs.AddRange(ffPass2Args);
                    stageArgs.Add("-f");
                    stageArgs.Add("mpeg1video");
                    stageArgs.Add(outputFile + ".m1v");

                    plan.AddStage(new ExecutionStage(ffmpegPath, stageArgs.ToArray()));
                }
                plan.AddTemporaryFile(outputFile + ".m1v");
                plans.Add(plan);
            }

            // M1V import
            {
                ExecutionPlan plan = new ExecutionPlan();
                List<string> args = new List<string>();
                args.Add("importm1v");
                args.Add(outputFile + ".m1v");
                args.Add(outputFile);
                if (FullRange)
                    args.Add("1");
                else
                    args.Add("0");
                plan.AddStage(new ExecutionStage(lwmuxPath, args.ToArray()));

                plan.CompletionCallback = pcd;
                plan.AddCleanupFile(outputFile + ".m1v");
                plans.Add(plan);
            }
        }

        public MPEG1VideoSettings()
        {
            Quality = 20;
            UseQuality = true;
            BitrateMaxPresetIndex = 3;
            NumBFrames = 5;
            BStrategy = 1;
            MotionEstimationPresetIndex = 0;
            BitrateMinPresetIndex = 3;
        }
    }
}
