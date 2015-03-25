using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace lwfe
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

    public class CELTAudioCodec : ICodec
    {
        string ICodec.CodecID { get { return "CELT"; } }
        string ICodec.Name { get { return "Opus Custom (CELT)"; } }
        IExecutionPlanSettings ICodec.CreateExecutionPlanSettings() { return new CELTAudioSettings(); }
        Control ICodec.CreateCodecControl() { return new CELTAudioOptionsControl(); }
    }

    public class CodecRepository
    {
        public static ICodec[] VideoCodecs = new ICodec[]
        {
            new MPEG1VideoCodec(),
        };
        public static ICodec[] AudioCodecs = new ICodec[]
        {
            new CELTAudioCodec(),
        };
    }
}
