class Block:
    def __init__(self, block_id=None, block_type=None, text=None, next_block_id=None, 
                 true_block_id=None, false_block_id=None, position=None):
        self.id = block_id
        self.type = block_type
        self.text = text
        self.next_block_id = next_block_id
        self.true_block_id = true_block_id
        self.false_block_id = false_block_id
        self.position = position if position else (0, 0)  # Using tuple (x, y) instead of Point
