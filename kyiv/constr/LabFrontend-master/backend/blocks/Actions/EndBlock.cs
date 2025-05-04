using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LabBackend.Blocks.Actions
{
    public class EndBlock : AbstractBlock
    {
        public EndBlock(int Id) : base(Id, "")
        {
            this.NameBlock = "end";
        }

        public override void Execute()
        {
            Console.WriteLine("Start block executed");
        }
    }
}
