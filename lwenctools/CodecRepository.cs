using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwenctools
{
    public interface ICodec
    {
        IExecutionPlanSettings CreateExecutionPlanSettings();
        Control CreateCodecControl();
        string Name { get; }
        string CodecID { get; }
    }

    public class MPEG1VideoCodec : ICodec
    {
        string ICodec.CodecID { get { return "M1V"; } }
        string ICodec.Name { get { return "MPEG-1 Video"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new MPEG1VideoSettings(); }
        Control ICodec.CreateCodecControl() { return new MPEG1VideoOptionsControl(); }
    }

    public class ROQVideoCodec : ICodec
    {
        string ICodec.CodecID { get { return "ROQ"; } }
        string ICodec.Name { get { return "RoQ Video"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new ROQVideoSettings(); }
        Control ICodec.CreateCodecControl() { return new ROQVideoOptionsControl(); }
    }

    public class TheoraVideoCodec : ICodec
    {
        string ICodec.CodecID { get { return "Theora"; } }
        string ICodec.Name { get { return "lwtheora"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new TheoraVideoSettings(); }
        Control ICodec.CreateCodecControl() { return new TheoraVideoOptionsControl(); }
    }

    public class CELTAudioCodec : ICodec
    {
        string ICodec.CodecID { get { return "CELT"; } }
        string ICodec.Name { get { return "Opus Custom (CELT)"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new CELTAudioSettings(); }
        Control ICodec.CreateCodecControl() { return new CELTAudioOptionsControl(); }
    }

    public class MP2AudioCodec : ICodec
    {
        string ICodec.CodecID { get { return "MP2"; } }
        string ICodec.Name { get { return "MPEG Layer II"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new MP2AudioSettings(); }
        Control ICodec.CreateCodecControl() { return new MP2AudioOptionsControl(); }
    }

    public class ADPCMAudioCodec : ICodec
    {
        string ICodec.CodecID { get { return "ADPCM"; } }
        string ICodec.Name { get { return "4-bit ADPCM"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new ADPCMAudioSettings(); }
        Control ICodec.CreateCodecControl() { return new ADPCMAudioOptionsControl(); }
    }

    public class CodecRepository
    {
        public static ICodec[] VideoCodecs = new ICodec[]
        {
            new TheoraVideoCodec(),
            new MPEG1VideoCodec(),
            new ROQVideoCodec(),
        };
        public static ICodec[] AudioCodecs = new ICodec[]
        {
            new CELTAudioCodec(),
            new MP2AudioCodec(),
            new ADPCMAudioCodec(),
        };

        public static ICodec GetVideoCodec(string codecID)
        {
            foreach (ICodec codec in VideoCodecs)
                if (codec.CodecID == codecID)
                    return codec;
            return null;
        }

        public static ICodec GetAudioCodec(string codecID)
        {
            foreach (ICodec codec in AudioCodecs)
                if (codec.CodecID == codecID)
                    return codec;
            return null;
        }
    }
}
