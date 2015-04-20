using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwenctools
{
    public delegate void PlanCompletionDelegate();

    public interface IExecutionPlanSettings
    {
        void SaveToXml(XmlElement xml);
        void LoadFromXml(XmlElement xml);

        bool Validate(List<string> outErrors);

        void CreateCommands(List<ExecutionPlan> plan, Dictionary<string, object> externalSettings, PlanCompletionDelegate completionDelegate);
    }
}
