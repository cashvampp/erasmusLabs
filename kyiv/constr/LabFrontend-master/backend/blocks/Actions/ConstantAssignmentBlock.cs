using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Actions
{
    // Клас для присвоєння значення V=C
    public class ConstantAssignmentBlock : AbstractBlock
    {
        public ConstantAssignmentBlock(int Id, string data) : base(Id, data)
        {
            this.NameBlock = "ConstantAssignmentBlock";
        }
        private bool IsValidAssignment(string data)
        {
            return Regex.IsMatch(data, @"^([a-zA-Z_]\w*)=(\d+)$");
        }

        public override void Execute(string programmingLanguage, int amountTabs)
        {
            if (!IsValidAssignment(this.Data))
            {
                Console.WriteLine("Invalid assignment format");
                return;
            }

            string[] parts = this.Data.Split('=');
            string variableName = parts[0].Trim();
            string value = parts[1].Trim();

            Console.WriteLine($"Executing {this.Id} \"{this.NameBlock}\": {this.Data}");
            switch (programmingLanguage)
            {
                case "C":
                case "C++":
                case "C#":
                case "Java":
                    Console.WriteLine($"{new string('\t', amountTabs)}int {variableName} = {value};");
                    break;
                case "Python":
                    Console.WriteLine($"{new string('\t', amountTabs)}{variableName} = {value}");
                    break;
                default:
                    Console.WriteLine("Unknown programming language");
                    return;
            }
        }
    }
}
