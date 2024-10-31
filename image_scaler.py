import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import os
from skimage import io, img_as_float

def load_image(image_path):
    # Load and preprocess the image
    image = io.imread(image_path)
    # Ensures pixel values are between 0 and 1
    image = img_as_float(image).astype(np.float64)
    # Read as RGB, not RGBA
    if image.shape[2] > 3:
        image = image[:, :, :3]
    print("Image loaded and normalized.")
    return image

def scale(image, scaling_factor):
    image_height, image_width, _ = image.shape
    scaled_image = Image.fromarray(image).resize((int(image_width * scaling_factor), int(image_height * scaling_factor)))
    return np.asarray(scaled_image)

def scale(image, width, height):
    scaled_image = Image.fromarray(image).resize((width, height))
    return np.asarray(scaled_image)

if __name__ == '__main__':

    input_folder = './data/noise'
    name = 'wood.png'
    file_name = os.path.join(input_folder, name)

    img = Image.open(file_name).convert('RGB')
    input_image = np.asarray(img)
    scaling_factor = 0.25

    # scaled_image = scale(input_image, scaling_factor)
    scaled_image = scale(input_image, 256, 256)

    # print the dimensions of the images
    print("Original Image Dimensions: ", input_image.shape)
    print("Scaled Image Dimensions: ", scaled_image.shape)
    
    # Plot the images
    fig, axes = plt.subplots(1, 2, figsize=(10, 5))
    axes[0].imshow(input_image)
    axes[0].set_title('Input Image')
    axes[1].imshow(scaled_image)
    axes[1].set_title('Scaled Image')
    plt.show()

    output_name = name.split('.')[0] + '_scaled.png'
    Image.fromarray(scaled_image).save(output_name)



