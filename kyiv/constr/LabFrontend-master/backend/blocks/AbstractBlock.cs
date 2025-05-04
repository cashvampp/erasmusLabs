using LabBackend.Utils.Interfaces;
using LabBackend.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace LabBackend.Utils.Abstract
{
    public abstract class AbstractBlock : ICloneable
    {
        public string NameBlock { get; set; }
        public int Id { get; set; }
        public string Data { get; set; }
        public AbstractBlock[] Next { get; set; }

        public AbstractBlock(int id, string data)
        {
            IIdentifiable identifiable = new Md5Manager();

            this.Id = id;
            this.Data = data;
            this.Next = Array.Empty<AbstractBlock>();
        }
        public AbstractBlock(List<AbstractBlock> UIArray)
        {
            IIdentifiable identifiable = new Md5Manager();
            //Id = identifiable.generateID();
            this.Id = 0;
            this.Next = UIArray.ToArray();
        }
        public virtual object Clone()
        {
            return (AbstractBlock)MemberwiseClone();
        }
        public virtual string getNameBlock()
        {
            return NameBlock;
        }
        public virtual int getId()
        {
            return Id;
        }
        public virtual void setData(string data)
        {
            Data = data;
        }
        public virtual string getData()
        {
            return Data;
        }
        public virtual void Execute()
        {
            Console.WriteLine($"Executing AbstractBlock: {NameBlock}, Data: {Data}\n");

            foreach (var nextBlock in Next)
            {
                if (nextBlock != null)
                {
                    nextBlock.Execute();
                }
            }
        }
        public virtual void Execute(string programmingLanguage, int amountTabs)
        {
            Console.WriteLine($"Executing AbstractBlock: {NameBlock}, Data: {Data}\n");

            foreach (var nextBlock in Next)
            {
                if (nextBlock != null)
                {
                    nextBlock.Execute(programmingLanguage, amountTabs);
                }
            }
        }
    }
}
