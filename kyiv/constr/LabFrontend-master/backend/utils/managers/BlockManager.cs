using LabBackend.Blocks.Actions;
using LabBackend.Utils.Abstract;
using LabBackend.Utils.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WpfApp2.frontend.blocks;

namespace LabBackend.Utils
{
    public class BlockManager : IManager
    {
        public void SetLink(List<AbstractBlock> uiBlocks, int fromID, int[] toID)
        {
            foreach (var mainBlock in uiBlocks)
            {
                if (mainBlock.getId() == fromID)
                {
                    List<AbstractBlock> buffer = new List<AbstractBlock>();

                    foreach (var slaveBlockID in toID)
                    {
                        foreach (var slaveBlock in uiBlocks)
                        {
                            if (slaveBlock.getId() == slaveBlockID)
                            {
                                buffer.Add(slaveBlock);
                            }
                        }
                    }
                    mainBlock.Next = buffer.ToArray();
                }
            }
        }

        public void RemoveLink(List<AbstractBlock> uiBlocks, int fromID, int[] toID)
        {
            foreach (var mainBlock in uiBlocks)
            {
                if (mainBlock.getId() == fromID)
                {
                    List<AbstractBlock> buffer = new List<AbstractBlock>(mainBlock.Next);

                    foreach (var slaveBlockID in toID)
                    {
                        buffer.RemoveAll(block => block.getId() == slaveBlockID);
                    }
                    mainBlock.Next = buffer.ToArray();
                }
            }
        }

        public List<AbstractBlock> GetLinkedBlocks(List<AbstractBlock> uiBlocks)
        {
            List<AbstractBlock> result = new List<AbstractBlock>();
            HashSet<int> visited = new HashSet<int>();

            AbstractBlock startBlock = null;
            foreach (var block in uiBlocks)
            {
                if (block.getNameBlock() == "start")
                {
                    startBlock = block;
                    break;
                }
            }

            if (startBlock == null)
            {
                return result;
            }

            void Traverse(AbstractBlock currentBlock)
            {
                if (currentBlock == null || visited.Contains(currentBlock.getId()))
                {
                    return;
                }

                visited.Add(currentBlock.getId());
                result.Add(currentBlock);

                foreach (var nextBlock in currentBlock.Next)
                {
                    Traverse(nextBlock);
                }
            }

            Traverse(startBlock);

            return result;
        }
        public List<Block> getLinkedFrontendBlocks(List<Block> blocksRAWFrontend)
        {
            List<Block> result = new List<Block>();
            Block startBlock = blocksRAWFrontend[0];
            Block endBlock = blocksRAWFrontend[1];

            if (startBlock.NextBlockId == null &&
                startBlock.TrueBlockId == null &&
                startBlock.FalseBlockId == null)
            {
                result.Add(startBlock);
                return result;
            }

            result.Add(startBlock);
            result.Add(endBlock);

            blocksRAWFrontend.Remove(startBlock);
            blocksRAWFrontend.Remove(endBlock);

            foreach (var currentBlock in blocksRAWFrontend)
            {

                if (currentBlock.NextBlockId != null ||
                    currentBlock.TrueBlockId != null ||
                    currentBlock.FalseBlockId != null ||
                    blocksRAWFrontend.Any(block => block.Id == block.Id))
                {
                    result.Add(currentBlock);
                }
            }

            return result;
        }
        public AbstractBlock GetBlockById(List<AbstractBlock> uiLinkedBlocks, int Id)
        {
            foreach (var block in uiLinkedBlocks)
            {
                if (block.getId() == Id)
                {
                    return block;
                }
            }
            return null;
        }
        public Dictionary<int, Dictionary<int, bool>> createAdjacencyMatrix(List<Block> linkedFrontendBlocks)
        {
            Dictionary<int, Dictionary<int, bool>> matrix = new Dictionary<int, Dictionary<int, bool>>();
            Stack<int> buffer = new Stack<int>();
            List<Block> massive = new List<Block>();

            Block startBlock = linkedFrontendBlocks[0];

            int newId = 1;

            void Traverse(Block currentBlock)
            {
                if (currentBlock.Type != "if")
                {
                    Block nextBlock = linkedFrontendBlocks
                        .Select(block => block)
                        .Where(block => block.Id == currentBlock.NextBlockId)
                        .FirstOrDefault();
                    currentBlock.Id = newId;
                    massive.Add(currentBlock);

                    if (currentBlock.Type == "end")
                    {
                        return;
                    }

                    newId++;

                    if (nextBlock != null)
                    {
                        currentBlock.NextBlockId = newId;

                        Traverse(nextBlock);
                    }
                    return;
                }
                else
                {
                    Block trueBlock = linkedFrontendBlocks
                       .Select(block => block)
                       .Where(block => block.Id == currentBlock.TrueBlockId)
                       .FirstOrDefault();
                    Block falseBlock = linkedFrontendBlocks
                       .Select(block => block)
                       .Where(block => block.Id == currentBlock.FalseBlockId)
                       .FirstOrDefault();

                    currentBlock.Id = newId;
                    newId++;

                    massive.Add(currentBlock);

                    if (currentBlock.TrueBlockId != null)
                    {
                        currentBlock.TrueBlockId = newId;
                        Traverse(trueBlock);
                    }
                    if (currentBlock.FalseBlockId != null)
                    {
                        currentBlock.FalseBlockId = newId;
                        Traverse(falseBlock);
                    }

                    return;
                }

            }

            Traverse(startBlock);

            foreach (Block block in massive)
            {
                matrix[block.Id] = new Dictionary<int, bool>();
                if (block.NextBlockId.HasValue)
                {
                    matrix[block.Id][block.NextBlockId.Value] = true;
                }
                if (block.TrueBlockId.HasValue)
                {
                    matrix[block.Id][block.TrueBlockId.Value] = true;
                }
                if (block.FalseBlockId.HasValue)
                {
                    matrix[block.Id][block.FalseBlockId.Value] = true;
                }
            }
            return matrix;
        }
    }
}
