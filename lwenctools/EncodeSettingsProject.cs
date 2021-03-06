﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public class EncodeSettingsProject
    {
        public class AudioStreamSettings
        {
            public Dictionary<string,IExecutionPlanSettings> AudioCodecSettings { get; private set; }
            public string EncodeInputFile { get; set; }
            public string EncodeOutputFile { get; set; }
            public string IntermediateFile { get; set; }
            public string MetaID { get; set; }
            public bool UseIntermediate { get; set; }

            public AudioStreamSettings()
            {
                AudioCodecSettings = new Dictionary<string, IExecutionPlanSettings>();
            }
        }

        public Dictionary<string,IExecutionPlanSettings> VideoCodecSettings { get; private set; }
        public List<AudioStreamSettings> AudioStreams { get; private set; }

        public string VideoCodecID { get; set; }
        public string VideoIntermediateFile { get; set; }
        public string VideoEncodeInputFile { get; set; }
        public string VideoEncodeOutputFile { get; set; }
        public string MuxOutputFile { get; set; }
        public bool VideoUseIntermediate { get; set; }
        public string AudioCodecID { get; set; }
        public int SelectedAudioStreamIndex { get; set; }

        public EncodeSettingsProject()
        {
            AudioCodecID = "CELT";
            VideoCodecID = "Theora";
            AudioStreams = new List<AudioStreamSettings>();
            VideoCodecSettings = new Dictionary<string, IExecutionPlanSettings>();
            SelectedAudioStreamIndex = -1;
        }

        public bool LoadFromXml(XmlDocument xmlDocument)
        {
            XmlElement projectNode = xmlDocument.DocumentElement;
            if (projectNode.Name != "LWFEProject")
                return false;

            VideoCodecID = projectNode.GetAttribute("VideoCodecID");
            VideoIntermediateFile = projectNode.GetAttribute("VideoIntermediateFile");
            VideoEncodeInputFile = projectNode.GetAttribute("VideoEncodeInputFile");
            VideoEncodeOutputFile = projectNode.GetAttribute("VideoEncodeOutputFile");
            VideoIntermediateFile = projectNode.GetAttribute("VideoIntermediateFile");
            VideoUseIntermediate = (projectNode.GetAttribute("VideoUseIntermediate") == "True");
            AudioCodecID = projectNode.GetAttribute("AudioCodecID");
            {
                int temp;
                if (int.TryParse(projectNode.GetAttribute("SelectedAudioStreamIndex"), out temp))
                    SelectedAudioStreamIndex = temp;
            }

            MuxOutputFile = projectNode.GetAttribute("MuxOutputFile");

            {
                foreach (XmlNode vCodecSettingsListNode in projectNode.GetElementsByTagName("VideoCodecsSettingsList"))
                {
                    VideoCodecSettings = new Dictionary<string, IExecutionPlanSettings>();
                    foreach (XmlNode vCodecNode in ((XmlElement)vCodecSettingsListNode).GetElementsByTagName("VideoCodecSettings"))
                    {
                        string codecID = ((XmlElement)vCodecNode).GetAttribute("VideoCodecID");

                        ICodec codec = CodecRepository.GetVideoCodec(codecID);
                        if (codec == null)
                            return false;
                        IExecutionPlanSettings xps = codec.CreateExecutionPlanSettings();
                        xps.LoadFromXml((XmlElement)vCodecNode);
                        VideoCodecSettings[codecID] = xps;
                    }
                }
            }

            AudioStreams = new List<AudioStreamSettings>();
            foreach (XmlNode audioStreamsNode in projectNode.GetElementsByTagName("AudioStreamsList"))
            {
                foreach (XmlNode aStreamNode in ((XmlElement)audioStreamsNode).GetElementsByTagName("AudioStream"))
                {
                    AudioStreamSettings audioStream = new AudioStreamSettings();

                    XmlElement aStreamElement = (XmlElement)aStreamNode;
                    audioStream.UseIntermediate = (aStreamElement.GetAttribute("UseIntermediate") != "False");
                    audioStream.IntermediateFile = aStreamElement.GetAttribute("IntermediateFile");
                    audioStream.EncodeInputFile = aStreamElement.GetAttribute("EncodeInputFile");
                    audioStream.EncodeOutputFile = aStreamElement.GetAttribute("EncodeOutputFile");
                    audioStream.MetaID = aStreamElement.GetAttribute("MetaID");
                    foreach (XmlNode aCodecSettingsListNode in aStreamElement.GetElementsByTagName("AudioCodecSettingsList"))
                    {
                        foreach (XmlNode aCodecNode in ((XmlElement)aCodecSettingsListNode).GetElementsByTagName("AudioCodecSettings"))
                        {
                            XmlElement aCodecElement = (XmlElement)aCodecNode;
                            string codecID = aCodecElement.GetAttribute("AudioCodecID");

                            ICodec codec = CodecRepository.GetAudioCodec(codecID);
                            if (codec == null)
                                return false;

                            IExecutionPlanSettings xps = codec.CreateExecutionPlanSettings();
                            xps.LoadFromXml(aCodecElement);
                            audioStream.AudioCodecSettings[codecID] = xps;
                        }
                    }

                    AudioStreams.Add(audioStream);
                }
            }

            return true;
        }

        public XmlDocument SaveToXml()
        {
            XmlDocument doc = new XmlDocument();
            XmlElement projectNode = doc.CreateElement("LWFEProject");
            doc.AppendChild(projectNode);

            projectNode.SetAttribute("VideoCodecID", VideoCodecID);
            projectNode.SetAttribute("VideoIntermediateFile", VideoIntermediateFile);
            projectNode.SetAttribute("VideoEncodeInputFile", VideoEncodeInputFile);
            projectNode.SetAttribute("VideoEncodeOutputFile", VideoEncodeOutputFile);
            projectNode.SetAttribute("VideoIntermediateFile", VideoIntermediateFile);
            projectNode.SetAttribute("VideoUseIntermediate", VideoUseIntermediate ? "True" : "False");
            projectNode.SetAttribute("AudioCodecID", AudioCodecID);
            projectNode.SetAttribute("SelectedAudioStreamIndex", SelectedAudioStreamIndex.ToString());
            projectNode.SetAttribute("MuxOutputFile", MuxOutputFile);

            {
                XmlElement vCodecSettingsListNode = doc.CreateElement("VideoCodecsSettingsList");
                foreach (KeyValuePair<string, IExecutionPlanSettings> vcs in VideoCodecSettings)
                {
                    XmlElement vCodecNode = doc.CreateElement("VideoCodecSettings");

                    vCodecNode.SetAttribute("VideoCodecID", vcs.Key);
                    vcs.Value.SaveToXml(vCodecNode);

                    vCodecSettingsListNode.AppendChild(vCodecNode);
                }
                projectNode.AppendChild(vCodecSettingsListNode);
            }

            {
                XmlElement audioStreamsNode = doc.CreateElement("AudioStreamsList");
                foreach (AudioStreamSettings audioStream in AudioStreams)
                {
                    XmlElement aStreamNode = doc.CreateElement("AudioStream");
                    aStreamNode.SetAttribute("UseIntermediate", audioStream.UseIntermediate ? "True" : "False");
                    aStreamNode.SetAttribute("IntermediateFile", audioStream.IntermediateFile);
                    aStreamNode.SetAttribute("EncodeInputFile", audioStream.EncodeInputFile);
                    aStreamNode.SetAttribute("EncodeOutputFile", audioStream.EncodeOutputFile);
                    aStreamNode.SetAttribute("MetaID", audioStream.MetaID);
                    {
                        XmlElement aCodecSettingsListNode = doc.CreateElement("AudioCodecSettingsList");
                        {
                            foreach (KeyValuePair<string, IExecutionPlanSettings> acs in audioStream.AudioCodecSettings)
                            {
                                XmlElement aCodecNode = doc.CreateElement("AudioCodecSettings");
                                aCodecNode.SetAttribute("AudioCodecID", acs.Key);
                                acs.Value.SaveToXml(aCodecNode);
                                aCodecSettingsListNode.AppendChild(aCodecNode);
                            }
                        }
                        aStreamNode.AppendChild(aCodecSettingsListNode);
                    }
                    audioStreamsNode.AppendChild(aStreamNode);
                }
                projectNode.AppendChild(audioStreamsNode);
            }

            return doc;
        }

        public void CreateDefaultSettings(Dictionary<string, object> settings)
        {
            string currentExePath = System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName;
            string currentExeDir = currentExePath.Substring(0, currentExePath.LastIndexOf('\\'));
            string lwmuxPath = currentExeDir + "\\lwmux.exe";
            string lwrerangePath = currentExeDir + "\\lwrerange.exe";
            string lwroqencPath = currentExeDir + "\\lwroqenc.exe";
            string lwthencPath = currentExeDir + "\\lwthenc.exe";
            string ffmpegPath = currentExeDir + "\\ffmpeg\\bin\\ffmpeg.exe";

            settings["lwmuxPath"] = lwmuxPath;
            settings["lwrerangePath"] = lwrerangePath;
            settings["ffmpegPath"] = ffmpegPath;
            settings["lwroqencPath"] = lwroqencPath;
            settings["lwthencPath"] = lwthencPath;
        }

        public bool Validate(List<string> errors)
        {
            bool isOK = true;

            if (VideoUseIntermediate)
            {
                if (VideoIntermediateFile == "" || VideoIntermediateFile == null)
                {
                    errors.Add("Video intermediate file was not specified");
                    isOK = false;
                }
                else if (!System.IO.File.Exists(VideoIntermediateFile))
                {
                    errors.Add("Video intermediate file does not exist");
                    isOK = false;
                }
            }
            else
            {
                if (!VideoCodecSettings[VideoCodecID].Validate(errors))
                    isOK = false;
                if (VideoEncodeInputFile == "" || VideoEncodeInputFile == null)
                {
                    errors.Add("Video input file was not specified");
                    isOK = false;
                }
                else if (!System.IO.File.Exists(VideoEncodeInputFile))
                {
                    errors.Add("Video input file does not exist");
                    isOK = false;
                }
            }

            int streamIndex = 0;
            foreach (EncodeSettingsProject.AudioStreamSettings audioStream in AudioStreams)
            {
                streamIndex = streamIndex + 1;
                string streamPrefix = "Audio stream " + streamIndex.ToString();
                if (audioStream.UseIntermediate)
                {
                    string checkFile = audioStream.IntermediateFile;
                    if (checkFile == "" || checkFile == null)
                    {
                        errors.Add(streamPrefix + " intermediate file was not specified");
                        isOK = false;
                    }
                    else if (!System.IO.File.Exists(VideoEncodeInputFile))
                    {
                        errors.Add(streamPrefix + " intermediate file does not exist");
                        isOK = false;
                    }
                }
                else
                {
                    if (!audioStream.AudioCodecSettings[AudioCodecID].Validate(errors))
                        isOK = false;

                    string checkFile = audioStream.EncodeInputFile;
                    if (checkFile == "" || checkFile == null)
                    {
                        errors.Add(streamPrefix + " input file was not specified");
                        isOK = false;
                    }
                    else if (!System.IO.File.Exists(VideoEncodeInputFile))
                    {
                        errors.Add(streamPrefix + " input file does not exist");
                        isOK = false;
                    }
                }
            }

            if (MuxOutputFile == null || MuxOutputFile.Length == 0)
            {
                errors.Add("No final output file was specified");
                isOK = false;
            }
            return isOK;
        }

        public List<ExecutionPlan> CreateExecutionPlans(Dictionary<string, object> inSettings)
        {
            Dictionary<string, object> settings = new Dictionary<string, object>();
            foreach (KeyValuePair<string, object> kvp in inSettings)
                settings.Add(kvp.Key, kvp.Value);

            List<string> audioStreamPaths = new List<string>();
            string videoStreamPath = null;

            List<ExecutionPlan> ePlans = new List<ExecutionPlan>();
            if (!VideoUseIntermediate)
            {
                settings["InputFile"] = VideoEncodeInputFile;
                settings["OutputFile"] = VideoEncodeOutputFile;
                VideoCodecSettings[VideoCodecID].CreateCommands(ePlans, settings, delegate()
                {
                    VideoIntermediateFile = VideoEncodeOutputFile;
                    VideoUseIntermediate = true;
                });

                videoStreamPath = VideoEncodeOutputFile;
            }
            else
            {
                videoStreamPath = VideoIntermediateFile;
            }

            foreach (EncodeSettingsProject.AudioStreamSettings audioStream in AudioStreams)
            {
                if (!audioStream.UseIntermediate)
                {
                    settings["InputFile"] = audioStream.EncodeInputFile;
                    settings["OutputFile"] = audioStream.EncodeOutputFile;
                    settings["MetaID"] = audioStream.MetaID;

                    EncodeSettingsProject.AudioStreamSettings currentStream = audioStream;
                    audioStream.AudioCodecSettings[AudioCodecID].CreateCommands(ePlans, settings, delegate()
                    {
                        audioStream.IntermediateFile = audioStream.EncodeOutputFile;
                        audioStream.UseIntermediate = true;
                    });

                    audioStreamPaths.Add(audioStream.EncodeOutputFile);
                }
                else
                {
                    audioStreamPaths.Add(audioStream.IntermediateFile);
                }
            }

            // Create finalize task
            List<string> muxArgs = new List<string>();
            muxArgs.Add("finalize");
            muxArgs.Add(videoStreamPath);
            muxArgs.Add(MuxOutputFile);
            muxArgs.Add("10000");
            muxArgs.Add(audioStreamPaths.Count.ToString());
            muxArgs.AddRange(audioStreamPaths);

            {
                ExecutionPlan plan = new ExecutionPlan();
                ExecutionStage stage = new ExecutionStage((string)settings["lwmuxPath"], muxArgs.ToArray());
                plan.AddStage(stage);
                ePlans.Add(plan);
            }

            return ePlans;
        }
    }
}
