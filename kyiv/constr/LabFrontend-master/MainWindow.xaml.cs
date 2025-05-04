using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using LabBackend.Blocks.Actions;
using LabBackend.Blocks.Conditions;
using LabBackend.Utils;
using LabBackend.Utils.Abstract;
using LabBackend.Utils.Interfaces;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using WpfApp2.frontend.blocks;
using WpfApp2.frontend.utils;

namespace BlockLinkingApp
{
    public partial class MainWindow : Window
    {
        UIManager uiManager;
        public MainWindow()
        {
            InitializeComponent();
            uiManager = new UIManager(WorkspaceCanvas);

            InitWorkspace();
            InitComboBox();
        }

        private void InitWorkspace()
        {
            var startBlock = new Block { Id = 1, Type = "start", Text = "Start", NextBlockId = null, Position = new Point(50, 50) };
            var endBlock = new Block { Id = 2, Type = "end", Text = "End", NextBlockId = null, Position = new Point(50, 150) };

            uiManager.blocks.Add(startBlock);
            uiManager.blocks.Add(endBlock);

            StartBlock.Tag = startBlock;
            EndBlock.Tag = endBlock;

            AddBlockToCanvas(startBlock);
            AddBlockToCanvas(endBlock);
        }

        private void InitComboBox()
        {
            LanguageComboBox.Items.Add("C");
            LanguageComboBox.Items.Add("C++");
            LanguageComboBox.Items.Add("C#");
            LanguageComboBox.Items.Add("Java");
            LanguageComboBox.Items.Add("Python");
            LanguageComboBox.SelectedIndex = 0;
        }

        private void AddBlockButton_Click(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            string blockType = button?.Tag?.ToString();

            var newBlock = new Block
            {
                Id = uiManager.blocks.Count + 1,
                Type = blockType,
                Text = $"{blockType} Block {uiManager.blocks.Count + 1}",
                NextBlockId = null,
                Position = new Point(10, 10)
            };

            if (blockType == "if")
            {
                newBlock.Text = "If Block";
                newBlock.TrueBlockId = null;
                newBlock.FalseBlockId = null;
            }

            uiManager.blocks.Add(newBlock);
            AddBlockToCanvas(newBlock);
        }

        private void AddBlockToCanvas(Block block)
        {
            var border = new Border
            {
                Width = 100,
                Height = 50,
                Background = GetBlockBackground(block.Type),
                Tag = block
            };

            var textBlock = new TextBlock
            {
                Text = block.Text,
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            border.Child = textBlock;

            Canvas.SetLeft(border, block.Position.X);
            Canvas.SetTop(border, block.Position.Y);

            border.MouseMove += Border_MouseMove;
            border.MouseDown += Border_MouseDown;

            uiManager.WorkspaceCanvas.Children.Add(border);
        }

        private void Border_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed && sender is Border border)
            {
                DragDrop.DoDragDrop(border, border, DragDropEffects.Move);
            }
        }

        private void Border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is Border border && border.Tag is Block block)
            {
                if (e.RightButton == MouseButtonState.Pressed)
                {
                    if (block.Type == "if")
                    {
                        HandleIfBlockRightClick(block); 
                    }
                    else
                    {
                        HandleOtherBlockRightClick(block);
                    }
                }
                else if (e.LeftButton == MouseButtonState.Pressed && uiManager.sourceBlock != null)
                {
                    HandleLeftClickOnBlock(block); 
                }
            }
        }

        private void HandleIfBlockRightClick(Block block)
        {
          
            var connectionChoiceWindow = new ConnectionChoiceWindow();
            bool? result = connectionChoiceWindow.ShowDialog();

            if (result == true)
            {
               
                if (connectionChoiceWindow.IsTrueSelected)
                {
                    uiManager.sourceBlock = block;
                    MessageBox.Show($"true link {block.Text}");
                }
               
                else
                {
                    uiManager.sourceBlock = block;
                    MessageBox.Show($"flase link {block.Text}");
                }
            }
        }

        
        private void HandleOtherBlockRightClick(Block block)
        {
            uiManager.sourceBlock = block;
            MessageBox.Show($"block chosen: {block.Text}");
        }

      
        private void HandleLeftClickOnBlock(Block block)
        {
            if (uiManager.sourceBlock != null && uiManager.sourceBlock != block)
            {
                if (uiManager.sourceBlock.Type == "if")
                {
                   
                    if (uiManager.sourceBlock.TrueBlockId == null)
                    {
                        uiManager.sourceBlock.TrueBlockId = block.Id;
                        uiManager.DrawArrow(uiManager.sourceBlock, block, Brushes.Green); 
                        MessageBox.Show($"connection True: {uiManager.sourceBlock.Text} -> {block.Text}");
                    }
                    else if (uiManager.sourceBlock.FalseBlockId == null)
                    {
                        uiManager.sourceBlock.FalseBlockId = block.Id;
                        uiManager.DrawArrow(uiManager.sourceBlock, block, Brushes.Red);
                        MessageBox.Show($"connection False: {uiManager.sourceBlock.Text} -> {block.Text}");
                    }
                }
                else
                {
                    uiManager.sourceBlock.NextBlockId = block.Id;
                    uiManager.DrawArrow(uiManager.sourceBlock, block, Brushes.Black);
                    MessageBox.Show($"connection created : {uiManager.sourceBlock.Text} -> {block.Text}");
                }

                uiManager.sourceBlock = null;
            }
        }

        private void WorkspaceCanvas_Drop(object sender, DragEventArgs e)
        {
            if (e.Data.GetData(typeof(Border)) is Border droppedBorder)
            {
                Point position = e.GetPosition(uiManager.WorkspaceCanvas);
                Canvas.SetLeft(droppedBorder, position.X - droppedBorder.ActualWidth / 2);
                Canvas.SetTop(droppedBorder, position.Y - droppedBorder.ActualHeight / 2);

                if (droppedBorder.Tag is Block block)
                {
                    block.Position = new Point(Canvas.GetLeft(droppedBorder), Canvas.GetTop(droppedBorder));
                }
            }
        }

        private void SaveAllButton_Click(object sender, RoutedEventArgs e)
        {
            var saveData = uiManager.getBlocks();
            var json = JsonConvert.SerializeObject(saveData, Formatting.Indented);

            var dialog = new Microsoft.Win32.SaveFileDialog
            {
                Filter = "JSON Files (*.json)|*.json|All Files (*.*)|*.*",
                DefaultExt = "json"
            };

            if (dialog.ShowDialog() == true)
            {
                File.WriteAllText(dialog.FileName, json);
                MessageBox.Show("дані діаграми збережені");
            }
        }

        private void LoadAllButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog
            {
                Filter = "JSON Files (*.json)|*.json|All Files (*.*)|*.*",
                DefaultExt = "json"
            };

            if (dialog.ShowDialog() == true)
            {
                uiManager.WorkspaceCanvas.Children.Clear();
                uiManager.blocks.Clear();

                var json = File.ReadAllText(dialog.FileName);
                var savedBlocks = JsonConvert.DeserializeObject<List<Block>>(json);

                foreach (var savedBlock in savedBlocks)
                {
                    var block = new Block
                    {
                        Id = savedBlock.Id,
                        Type = savedBlock.Type,
                        Text = savedBlock.Text,
                        NextBlockId = savedBlock.NextBlockId,
                        TrueBlockId = savedBlock.TrueBlockId,
                        FalseBlockId = savedBlock.FalseBlockId,
                        Position = new Point((double)savedBlock.Position.X, (double)savedBlock.Position.Y)
                    };

                    uiManager.blocks.Add(block);
                    AddBlockToCanvas(block);
                }

                foreach (var block in uiManager.blocks)
                {
                    if (block.NextBlockId.HasValue)
                    {
                        var toBlock = uiManager.blocks.Find(b => b.Id == block.NextBlockId.Value);
                        if (toBlock != null)
                        {
                            uiManager.DrawArrow(block, toBlock, Brushes.Black); 
                        }
                    }

                    if (block.TrueBlockId.HasValue)
                    {
                        var trueBlock = uiManager.blocks.Find(b => b.Id == block.TrueBlockId.Value);
                        if (trueBlock != null)
                        {
                            uiManager.DrawArrow(block, trueBlock, Brushes.Green); 
                        }
                    }

                    if (block.FalseBlockId.HasValue)
                    {
                        var falseBlock = uiManager.blocks.Find(b => b.Id == block.FalseBlockId.Value);
                        if (falseBlock != null)
                        {
                            uiManager.DrawArrow(block, falseBlock, Brushes.Red); 
                        }
                    }
                }

                MessageBox.Show("data success loaded");
            }
        }

        private void TranslateButton_Click(object sender, RoutedEventArgs e)
        {
            IManager blockManager = new BlockManager();

            List<Block> blocksRAWFrontend = uiManager.getBlocks();
            List<Block> linkedFrontendBlocks = blockManager.getLinkedFrontendBlocks(blocksRAWFrontend);
            Dictionary<int, Dictionary<int, bool>> blocksFrontend = blockManager.createAdjacencyMatrix(linkedFrontendBlocks);
            //foreach (Block block in blocksFrontend)
            //{
            //    switch (block.Type)
            //    {
            //        case "start":
            //            uiBlocksBackend.Add(new StartBlock(block.Id));
            //            break;
            //        case "end":
            //            uiBlocksBackend.Add(new EndBlock(block.Id));
            //            break;
            //        case "AssignmentBlock":
            //            uiBlocksBackend.Add(new AssignmentBlock(block.Id, block.Text));
            //            break;
            //        case "ConstantAssignmentBlock":
            //            uiBlocksBackend.Add(new ConstantAssignmentBlock(block.Id, block.Text));
            //            break;
            //        case "InputBlock":
            //            uiBlocksBackend.Add(new InputBlock(block.Id, block.Text));
            //            break;
            //        case "PrintBlock":
            //            uiBlocksBackend.Add(new PrintBlock(block.Id, block.Text));
            //            break;
            //        case "if":
            //            uiBlocksBackend.Add(new ConditionBlock(block.Id, block.Text));
            //            break;
            //    }
            //}

            //foreach (Block block in blocksFrontend)
            //{
            //    if (block.Type != "if")
            //    {
            //        if (block.NextBlockId != null)
            //        {
            //            blockManager.SetLink(uiBlocksBackend, block.Id, new int[] { (int)block.NextBlockId });
            //        }
            //        continue;
            //    }

            //    List<int> toIds = new List<int>();
            //    if (block.TrueBlockId != null)
            //    {
            //        toIds.Add((int)block.TrueBlockId);
            //    }
            //    if (block.FalseBlockId != null)
            //    {
            //        toIds.Add((int)block.FalseBlockId);
            //    }
            //    blockManager.SetLink(uiBlocksBackend, block.Id, toIds.ToArray());
            //}

            //List<AbstractBlock> uiLinkedBlocksBackend = blockManager.GetLinkedBlocks(uiBlocksBackend);

            //Dictionary<int, Dictionary<int, bool>> adjacencyMatrix = blockManager.CreateAdjacencyMatrix(uiLinkedBlocksBackend);
        }

        private Brush GetBlockBackground(string type)
        {
            return type switch
            {
                "AssignmentBlock" => Brushes.LightBlue,
                "ConstantAssignmentBlock" => Brushes.LightGreen,
                "InputBlock" => Brushes.LightYellow,
                "PrintBlock" => Brushes.LightPink,
                "start" => Brushes.LightSkyBlue,
                "end" => Brushes.LightGreen,
                "if" => Brushes.LightCoral, 
                _ => Brushes.LightGray
            };
        }
    }
}
