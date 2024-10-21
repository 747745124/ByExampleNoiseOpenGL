# write a function scale that takes an image and a scaling factor and returns a new image that is the original image scaled by the factor.
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

def scale(image, scaling_factor):
    image_height, image_width, _ = image.shape
    #simply use resize function from PIL
    scaled_image = Image.fromarray(image).resize((int(image_width * scaling_factor), int(image_height * scaling_factor)))
    return np.asarray(scaled_image)

if __name__ == '__main__':

    input_folder = 'data/noise'
    name = 'fire.png'
    file_name = os.path.join(input_folder, name)

    img = Image.open(file_name).convert('RGB')
    input_image = np.asarray(img)
    scaling_factor = 0.5
    scaled_image = scale(input_image, scaling_factor)

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



