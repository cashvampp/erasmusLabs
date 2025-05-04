import tkinter as tk
from frontend.models.blocks.block import Block

class UIManager:
    def __init__(self, workspace):
        self.blocks = []
        self.source_block = None
        self.workspace_canvas = workspace
        
    def find_visual_child(self, parent, block):
        """
        Find a widget in the workspace that represents the given block.
        
        In Python/Tkinter, we need to manage the references to widgets ourselves,
        as there's no direct equivalent to WPF's visual tree.
        
        Args:
            parent: The tkinter canvas
            block: The Block object to find
            
        Returns:
            The widget representing the block, or None if not found
        """
        # In a real implementation, you might want to maintain a dictionary mapping
        # blocks to their visual elements instead of searching
        for item in parent.find_all():
            tags = parent.gettags(item)
            if 'block' in tags and f'id:{block.id}' in tags:
                return item
        return None
    
    def draw_arrow(self, from_block, to_block, arrow_color):
        """
        Draw an arrow from one block to another
        
        Args:
            from_block: The source Block object
            to_block: The target Block object
            arrow_color: The color to use for the arrow
        """
        from_element = self.find_visual_child(self.workspace_canvas, from_block)
        to_element = self.find_visual_child(self.workspace_canvas, to_block)
        
        if from_element and to_element:
            # Get the coordinates of the elements
            from_coords = self.workspace_canvas.coords(from_element)
            to_coords = self.workspace_canvas.coords(to_element)
            
            # Calculate center points
            from_x = from_coords[0] + (from_coords[2] - from_coords[0]) / 2
            from_y = from_coords[1] + (from_coords[3] - from_coords[1]) / 2
            to_x = to_coords[0] + (to_coords[2] - to_coords[0]) / 2
            to_y = to_coords[1] + (to_coords[3] - to_coords[1]) / 2
            
            # Draw the line
            line = self.workspace_canvas.create_line(
                from_x, from_y, to_x, to_y, 
                fill=arrow_color, width=2, 
                arrow=tk.LAST  # Add an arrowhead at the end
            )
            
            # Tag the line so we can find it later if needed
            self.workspace_canvas.addtag_withtag(f"arrow:{from_block.id}:{to_block.id}", line)
    
    def get_blocks(self):
        """
        Get all blocks with their updated positions
        
        Returns:
            List of Block objects with updated positions
        """
        result = []
        
        for block in self.blocks:
            element = self.find_visual_child(self.workspace_canvas, block)
            
            if element:
                # Get the coordinates of the element
                coords = self.workspace_canvas.coords(element)
                
                # Update the block's position
                block.position = (coords[0], coords[1])
                
            result.append(block)
            
        return result
