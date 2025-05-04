using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Actions
{
    // Клас для присвоєння значення V1=V2
    public class AssignmentBlock : AbstractBlock
    {
        public AssignmentBlock(int Id, string data) : base(Id, data)
        {
            this.NameBlock = "AssignmentBlock";
        }
        private bool IsValidAssignment(string data)
        {
            return Regex.IsMatch(data, @"^([a-zA-Z_]\w*)=([a-zA-Z_]\w*)$");
        }
        public override void Execute(string programmingLanguage, int amountTabs)
        {
            if (!IsValidAssignment(this.Data))
            {
                Console.WriteLine("Invalid assignment format");
                return;
            }

            Console.WriteLine($"Executing {this.Id} \"{this.NameBlock}\": {this.Data}");
            string indent = new string('\t', amountTabs);
            string[] variables = this.Data.Split('=');
            if (variables.Length != 2)
            {
                throw new ArgumentException("Data should be in format 'V1=V2'");
            }
            string v1 = variables[0].Trim();
            string v2 = variables[1].Trim();

            StringBuilder code = new StringBuilder();
            switch (programmingLanguage)
            {
                case "C":
                    code.AppendLine($"{indent}strcpy({v1}, {v2});");
                    break;
                case "C++":
                case "C#":
                case "Java":
                    code.AppendLine($"{indent}{v1} = {v2};");
                    break;
                case "Python":
                    code.AppendLine($"{indent}{v1} = {v2}");
                    break;
                default:
                    throw new NotSupportedException($"Programming language '{programmingLanguage}' is not supported.");
            }
            Console.WriteLine(code.ToString());
        }
    }
}
