using LabBackend.Utils.Abstract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WpfApp2.frontend.blocks;

namespace LabBackend.Utils.Interfaces
{
    public interface IManager
    {
        public void SetLink(List<AbstractBlock> uiBlocks, int fromID, int[] toID);
        public void RemoveLink(List<AbstractBlock> uiBlocks, int fromID, int[] toID);
        public List<AbstractBlock> GetLinkedBlocks(List<AbstractBlock> uiBlocks);
        public List<Block> getLinkedFrontendBlocks(List<Block> blocksRAWFrontend);
        public AbstractBlock GetBlockById(List<AbstractBlock> uiLinkedBlocks, int Id);
        public Dictionary<int, Dictionary<int, bool>> createAdjacencyMatrix(List<Block> linkedFrontendBlocks);
    }
}
