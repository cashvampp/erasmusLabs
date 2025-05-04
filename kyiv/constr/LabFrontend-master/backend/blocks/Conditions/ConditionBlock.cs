using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Conditions
{
    // Клас для умовного блоку V==C або V<C
    public class ConditionBlock : AbstractBlock
    {
        public ConditionBlock(int Id, string data) : base(Id, data)
        {
            this.NameBlock = "Condition";
        }

        private bool IsValidCondition(string data)
        {
            return Regex.IsMatch(data, @"^[a-zA-Z_]\w*(==|<)\d+$");
        }

        public override void Execute(string programmingLanguage, int amountTabs)
        {
            if (!IsValidCondition(this.Data))
            {
                Console.WriteLine("Invalid condition format");
                return;
            }

            Console.WriteLine($"Executing {this.Id} \"{this.NameBlock}\": {this.Data}");

            string condition = "";

            switch (programmingLanguage)
            {
                case "Python":
                    condition = $"{new string('\t', amountTabs)}if {this.Data}:";
                    break;
                case "C":
                case "C++":
                case "C#":
                case "Java":
                    condition = $"{new string('\t', amountTabs)}if ({this.Data}) {{";
                    break;
                default:
                    Console.WriteLine("Unknown programming language");
                    return;
            }

            Console.WriteLine(condition);
        }
    }
}
