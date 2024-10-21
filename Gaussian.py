import numpy as np
import ot  # Python Optimal Transport library
from scipy.stats import norm
from skimage import io, img_as_float
from scipy.special import erfinv
import os

def get_mapping_from_plan(xs, xt, G, thr=1e-8):
    mapping = {}
    mx = G.max()

    for i in range(xs.shape[0]):
        for j in range(xt.shape[0]):
            if G[i, j] / mx > thr:
                mapping[i] = j
    
    return mapping

def gaussianize_image(input_image_path):
    image = io.imread(input_image_path)
    # Ensures pixel values are between 0 and 1
    image = img_as_float(image)  
    #read as rgb, not rgba
    image = image[:,:,:3]

    mean = 0.5
    variance = 1/36
    sigma = np.sqrt(variance)

    print("Started")
    # Assuming image is in the format (height, width, channels)
    height, width, channels = image.shape
    gaussian_variables = np.zeros_like(image)

    for channel in range(3):
        # Generate uniform random numbers
        uniform_random = np.random.rand(height, width)
        # Convert uniform to Gaussian using inverse CDF (ppf in scipy)
        gaussian_variables[:, :, channel] = mean + sigma * np.sqrt(2) * erfinv(2 * uniform_random - 1)

    # Flatten the image
    image_flat = image.reshape((image.shape[0] * image.shape[1], image.shape[2]))
    gaussian_flat = gaussian_variables.reshape((gaussian_variables.shape[0] * gaussian_variables.shape[1], gaussian_variables.shape[2]))
    print("Cost Matrix start")

    # Compute the cost matrix between the image and the Gaussian variables
    cost_matrix = ot.dist(image_flat, gaussian_flat, metric='sqeuclidean')
    print("Cost Matrix end")

    # compute EMD
    print("Calc EMD")
    transport_plan = ot.emd([],[], cost_matrix,numItermax=10000000, numThreads=10)
    print("Calc EMD end")
    
    # Apply the transport plan to obtain the Gaussianized image
    gaussianized_image = np.zeros_like(image_flat)
    for i in range(image_flat.shape[0]):
        gaussianized_image[i] = gaussian_flat[transport_plan[i].argmax()]

    gaussianized_image = gaussianized_image.reshape((height, width, channels))
        
    return gaussianized_image


input_image_path = 'data/noise/marble_128.png'
gaussianized_img = gaussianize_image(input_image_path)

# just keep the file name + _g, and put it under output folder
output_path = os.path.join('output', os.path.basename(input_image_path).split('.')[0] + '_g.png')
io.imsave(output_path, (gaussianized_img*255).astype('uint8'))




