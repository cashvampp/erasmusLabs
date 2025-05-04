using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Actions
{
    // Клас для команди PRINT V
    public class PrintBlock : AbstractBlock
    {
        public PrintBlock(int Id, string data) : base(Id, data)
        {
            this.NameBlock = "PrintBlock";
        }
        private bool IsValidVariableName(string variableName)
        {
            return Regex.IsMatch(variableName, @"^[a-zA-Z_]\w*$");
        }

        public override void Execute(string programmingLanguage, int amountTabs)
        {
            if (!IsValidVariableName(this.Data))
            {
                Console.WriteLine("Invalid variable name format");
                return;
            }

            Console.WriteLine($"Printing {this.Id} \"{this.NameBlock}\": {this.Data}");
            switch (programmingLanguage)
            {
                case "C":
                    Console.WriteLine($"{new string('\t', amountTabs)}printf(\"%s\", {this.Data});");
                    break;
                case "C++":
                    Console.WriteLine($"{new string('\t', amountTabs)}std::cout << {this.Data} << std::endl;");
                    break;
                case "C#":
                    Console.WriteLine($"{new string('\t', amountTabs)}Console.WriteLine({this.Data});");
                    break;
                case "Python":
                    Console.WriteLine($"{new string('\t', amountTabs)}print({this.Data});");
                    break;
                case "Java":
                    Console.WriteLine($"{new string('\t', amountTabs)}System.out.println({this.Data});");
                    break;
            }
        }
    }
}
