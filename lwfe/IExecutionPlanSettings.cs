using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace lwfe
{
    public delegate void PlanCompletionDelegate();

    public interface IExecutionPlanSettings
    {
        void SaveToXml(XmlElement xml);
        void LoadFromXml(XmlElement xml);

        void CreateCommands(List<ExecutionPlan> plan, Dictionary<string, object> externalSettings, PlanCompletionDelegate completionDelegate);
    }
}
