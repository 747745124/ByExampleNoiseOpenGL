import numpy as np
import ot  # Python Optimal Transport library
from scipy.stats import norm
from skimage import io, img_as_float
from scipy.special import erfinv,erf
import os
from tqdm import tqdm
from gaussianize import optimal_transport
from image_scaler import load_image

# this is the direct approach to mapping the gaussian image to the reference image
# Direct approach. A direct approach for inverting the transformation could be to use the optimal
# transport solver a second time to compute the best mapping between the blended Gaussian values
# to RGB values chosen randomly from the input image
def image_mapping(gaussian_image_path, reference_image_path, allow_diff_dimensions=False, sample_from_dest=False):
    # Load the input image
    gaussian_image = load_image(gaussian_image_path)
    # Load the reference image
    reference_image = load_image(reference_image_path)

    if not allow_diff_dimensions:
        assert gaussian_image.shape == reference_image.shape, "The images must have the same dimensions."

    result = optimal_transport(gaussian_image, reference_image, allow_diff_dimensions, sample_from_dest)
    return result


if __name__ == '__main__':
    #In OpenGL program, tick Gaussian Blended option, and take a screenshot
    gaussian_image_path = './result/granite_g_blended.png'
    #This is the source image
    reference_image_path = './data/noise/granite_256.png'

    gaussian_image = load_image(gaussian_image_path)
    reference_image = load_image(reference_image_path)

    # Since they are of different dimensions, we set allow_diff_dimensions to True, this will sample from the destination image (i.e. the input)
    result = optimal_transport(gaussian_image, reference_image, allow_diff_dimensions=True, sample_from_dest=True, batch_size=1000)
    io.imsave('result/granite_ot.png', (result*255).astype('uint8'))