using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwfe
{
    public interface ICodecSettingsControl
    {
        IExecutionPlanSettings GenerateSettings();
        void LoadFromSettings(IExecutionPlanSettings planSettings);
    }
}
