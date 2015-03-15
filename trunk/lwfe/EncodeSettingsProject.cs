using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwfe
{
    public class EncodeSettingsProject
    {
        public class AudioStreamSettings
        {
            public Dictionary<string,IExecutionPlanSettings> AudioCodecSettings { get; private set; }
            public string EncodeInputFile { get; set; }
            public string EncodeOutputFile { get; set; }
            public string IntermediateFile { get; set; }
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
        public bool VideoUseIntermediate { get; set; }
        public string AudioCodecID { get; set; }
        public int SelectedAudioStreamIndex { get; set; }

        public EncodeSettingsProject()
        {
            AudioCodecID = "CELT";
            VideoCodecID = "M1V";
            AudioStreams = new List<AudioStreamSettings>();
            VideoCodecSettings = new Dictionary<string, IExecutionPlanSettings>();
            SelectedAudioStreamIndex = -1;
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

            foreach(KeyValuePair<string,IExecutionPlanSettings> vcs in VideoCodecSettings)
            {
                XmlElement vCodecNode = doc.CreateElement("VideoCodecSettings");
                projectNode.AppendChild(vCodecNode);

                vCodecNode.SetAttribute("VideoCodecID", vcs.Key);
                vcs.Value.SaveToXml(vCodecNode);
            }

            return doc;
        }
    }
}
