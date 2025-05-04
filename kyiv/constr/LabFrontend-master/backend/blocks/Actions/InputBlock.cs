using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Actions
{
    // Клас для команди INPUT V
    public class InputBlock : AbstractBlock
    {
        public InputBlock(int Id, string data) : base(Id, data)
        {
            this.NameBlock = "InputBlock";
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

            Console.WriteLine($"Executing {this.Id} \"{this.NameBlock}\": {this.Data}");
            switch (programmingLanguage)
            {
                case "C":
                    Console.WriteLine($"{new string('\t', amountTabs)}scanf(\"%d\", &{this.Data});");
                    break;
                case "C++":
                    Console.WriteLine($"{new string('\t', amountTabs)}cin >> {this.Data};");
                    break;
                case "C#":
                    Console.WriteLine($"{new string('\t', amountTabs)}{this.Data} = int.Parse(Console.ReadLine());");
                    break;
                case "Python":
                    Console.WriteLine($"{new string('\t', amountTabs)}{this.Data} = int(input())");
                    break;
                case "Java":
                    Console.WriteLine($"{new string('\t', amountTabs)}{this.Data} = Integer.parseInt(scan.nextLine());");
                    break;
            }
        }
    }
}
