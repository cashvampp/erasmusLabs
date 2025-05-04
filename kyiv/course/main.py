import os
import shutil
import random
import yaml
from ultralytics import YOLO
from pathlib import Path
import cv2
import matplotlib.pyplot as plt
from tqdm import tqdm

class DroneDetectionTrainer:
    def __init__(self, dataset_path="drone_dataset", output_path="drone_model"):
        """
        Initialize the drone detection trainer
        
        Args:
            dataset_path: Path where the dataset will be stored/is stored
            output_path: Path where the model will be saved
        """
        self.dataset_path = Path(dataset_path)
        self.output_path = Path(output_path)
        
        # Create necessary directories
        os.makedirs(self.dataset_path, exist_ok=True)
        os.makedirs(self.output_path, exist_ok=True)
        
        # Dataset structure
        self.train_path = self.dataset_path / "train"
        self.val_path = self.dataset_path / "val"
        self.test_path = self.dataset_path / "test"
        
        for path in [self.train_path, self.val_path, self.test_path]:
            os.makedirs(path / "images", exist_ok=True)
            os.makedirs(path / "labels", exist_ok=True)
    
    def prepare_dataset(self, images_folder, annotations_folder=None, split_ratio=(0.7, 0.2, 0.1)):
        """
        Prepare dataset by splitting images and annotations into train/val/test sets
        
        Args:
            images_folder: Folder containing drone images
            annotations_folder: Folder containing YOLO format annotations
            split_ratio: Train/Val/Test split ratio
        """
        print("Preparing dataset...")
        images_path = Path(images_folder)
        if annotations_folder:
            annotations_path = Path(annotations_folder)
        
        # Get all image files
        image_files = list(images_path.glob("*.jpg")) + list(images_path.glob("*.png"))
        random.shuffle(image_files)
        
        # Calculate split sizes
        n_train = int(len(image_files) * split_ratio[0])
        n_val = int(len(image_files) * split_ratio[1])
        
        train_files = image_files[:n_train]
        val_files = image_files[n_train:n_train+n_val]
        test_files = image_files[n_train+n_val:]
        
        # Copy files to appropriate directories
        self._copy_files(train_files, self.train_path, annotations_path)
        self._copy_files(val_files, self.val_path, annotations_path)
        self._copy_files(test_files, self.test_path, annotations_path)
        
        print(f"Dataset prepared: {len(train_files)} training, {len(val_files)} validation, {len(test_files)} test images")
        
        # Create dataset YAML file
        self._create_dataset_yaml()
    
    def _copy_files(self, image_files, destination, annotations_path=None):
        """Helper method to copy images and annotations to destination folders"""
        for img_file in tqdm(image_files, desc=f"Copying to {destination.name}"):
            # Copy image
            shutil.copy(img_file, destination / "images" / img_file.name)
            
            # Copy annotation if exists
            if annotations_path:
                annotation_file = annotations_path / f"{img_file.stem}.txt"
                if annotation_file.exists():
                    shutil.copy(annotation_file, destination / "labels" / annotation_file.name)
    
    def _create_dataset_yaml(self):
        """Create YAML file for YOLOv8 training"""
        yaml_content = {
            'path': str(self.dataset_path.absolute()),
            'train': str(self.train_path / "images"),
            'val': str(self.val_path / "images"),
            'test': str(self.test_path / "images"),
            'names': {
                0: 'drone'
            }
        }
        
        with open(self.dataset_path / "dataset.yaml", 'w') as file:
            yaml.dump(yaml_content, file)
        
        print(f"Dataset YAML created at {self.dataset_path / 'dataset.yaml'}")
    
    def train_model(self, epochs=100, img_size=640, batch_size=16, pretrained_weights=None):
        """
        Train YOLOv8 model on the prepared dataset
        
        Args:
            epochs: Number of training epochs
            img_size: Image size for training
            batch_size: Batch size for training
            pretrained_weights: Path to pretrained weights (if None, uses COCO pretrained)
        """
        print("Starting model training...")
        
        # Initialize model
        if pretrained_weights:
            model = YOLO(pretrained_weights)
        else:
            model = YOLO('yolov8n.pt')  # using nano model as base
        
        # Train the model
        results = model.train(
            data=str(self.dataset_path / "dataset.yaml"),
            epochs=epochs,
            imgsz=img_size,
            batch=batch_size,
            name='drone_detection',
            patience=20,  # Early stopping patience
            device=0  # Use GPU if available
        )
        
        # Save the model
        best_model_path = Path('runs/detect/drone_detection/weights/best.pt')
        if best_model_path.exists():
            shutil.copy(best_model_path, self.output_path / "best_drone_model.pt")
            print(f"Best model saved to {self.output_path / 'best_drone_model.pt'}")
        
        return results
    
    def validate_model(self, model_path=None):
        """
        Validate the trained model
        
        Args:
            model_path: Path to the trained model
        """
        if model_path is None:
            model_path = self.output_path / "best_drone_model.pt"
        
        model = YOLO(model_path)
        results = model.val(data=str(self.dataset_path / "dataset.yaml"))
        
        print(f"Validation results: {results}")
        return results
    
    def export_model(self, format="onnx", model_path=None):
        """
        Export model to specified format for deployment
        
        Args:
            format: Format to export to ('onnx', 'tflite', etc.)
            model_path: Path to the trained model
        """
        if model_path is None:
            model_path = self.output_path / "best_drone_model.pt"
        
        model = YOLO(model_path)
        model.export(format=format)
        
        # Copy exported model to output directory
        exported_path = Path(str(model_path).replace('.pt', f'.{format}'))
        if exported_path.exists():
            shutil.copy(exported_path, self.output_path / f"drone_detection.{format}")
            print(f"Model exported to {self.output_path / f'drone_detection.{format}'}")
    
    def test_inference(self, image_path, model_path=None):
        """
        Test inference on a single image
        
        Args:
            image_path: Path to test image
            model_path: Path to the trained model
        """
        if model_path is None:
            model_path = self.output_path / "best_drone_model.pt"
        
        model = YOLO(model_path)
        results = model(image_path)
        
        # Visualize results
        img = cv2.imread(image_path)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        
        for result in results:
            boxes = result.boxes
            for box in boxes:
                x1, y1, x2, y2 = box.xyxy[0].tolist()
                conf = box.conf[0].item()
                cls = int(box.cls[0].item())
                
                cv2.rectangle(img, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                cv2.putText(img, f"Drone {conf:.2f}", (int(x1), int(y1-10)), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
        
        plt.figure(figsize=(10, 8))
        plt.imshow(img)
        plt.axis('off')
        plt.title("Drone Detection Result")
        plt.savefig(self.output_path / "test_result.jpg")
        plt.show()
        
        return results


class DatasetCreationHelper:
    """Helper class for creating drone detection datasets"""
    
    @staticmethod
    def video_to_frames(video_path, output_folder, frame_interval=30):
        """
        Extract frames from a video for dataset creation
        
        Args:
            video_path: Path to the input video
            output_folder: Folder to save extracted frames
            frame_interval: Extract one frame every N frames
        """
        os.makedirs(output_folder, exist_ok=True)
        
        video = cv2.VideoCapture(video_path)
        frame_count = 0
        saved_count = 0
        
        while True:
            success, frame = video.read()
            if not success:
                break
                
            if frame_count % frame_interval == 0:
                output_path = os.path.join(output_folder, f"frame_{saved_count:05d}.jpg")
                cv2.imwrite(output_path, frame)
                saved_count += 1
                
            frame_count += 1
        
        video.release()
        print(f"Extracted {saved_count} frames from video")
    
    @staticmethod
    def create_annotation_gui(image_folder, output_folder):
        """
        Basic GUI for annotating drone images (simplistic version)
        This is a placeholder - in practice, use a tool like labelImg or CVAT
        
        Args:
            image_folder: Folder containing drone images
            output_folder: Folder to save annotations
        """
        print("For real annotation, please use a proper tool like:")
        print("1. labelImg: https://github.com/tzutalin/labelImg")
        print("2. CVAT: https://github.com/opencv/cvat")
        print("3. Roboflow: https://roboflow.com")
        
        print("\nThese tools will help you create proper YOLO format annotations.")


# Usage example
if __name__ == "__main__":
    print("Drone Detection Training Pipeline")
    print("================================")
    
    # 1. Create dataset helper (if you need to extract frames from videos)
    data_helper = DatasetCreationHelper()
    
    # Uncomment to extract frames from a video
    # data_helper.video_to_frames("drone_video.mp4", "extracted_frames", frame_interval=30)
    
    # 2. Initialize trainer
    trainer = DroneDetectionTrainer(dataset_path="drone_dataset", output_path="drone_model")
    
    # 3. Prepare dataset (assuming you already have images and annotations)
    # trainer.prepare_dataset(images_folder="path/to/drone/images", 
    #                        annotations_folder="path/to/drone/annotations")
    
    # 4. Train model
    # results = trainer.train_model(epochs=100, img_size=640, batch_size=16)
    
    # 5. Validate model
    # trainer.validate_model()
    
    # 6. Export model for C++ deployment
    # trainer.export_model(format="onnx")
    
    # 7. Test on a single image
    # trainer.test_inference("test_drone_image.jpg")
    
    print("\nTo use this script:")
    print("1. Collect drone images and annotate them using a tool like labelImg")
    print("2. Organize them into folders")
    print("3. Uncomment and modify the relevant sections above")
    print("4. Run the script to train your drone detection model")