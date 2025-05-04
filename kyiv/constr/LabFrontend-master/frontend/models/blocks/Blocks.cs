using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace WpfApp2.frontend.blocks
{
    public class Block
    {
        public int Id { get; set; }
        public string Type { get; set; }
        public string Text { get; set; }
        public int? NextBlockId { get; set; }
        public int? TrueBlockId { get; set; }
        public int? FalseBlockId { get; set; }
        public Point Position { get; set; }
    }
}
