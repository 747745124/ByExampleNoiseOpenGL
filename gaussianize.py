import numpy as np
import ot  # Python Optimal Transport library
from scipy.stats import norm
from skimage import io, img_as_float
from scipy.special import erfinv
import os
from tqdm import tqdm

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

    # A batched implementation of the optimal transport, to avoid OOM
    def batch(sources, targets, batch_size = 10000):
        # Calculating the transport plan in batches
        num_source = sources.shape[0]
        num_target = targets.shape[0]
        
        # Split data into batches
        num_batches = max(num_source // batch_size + 1, num_target // batch_size + 1)
        source_batches = np.array_split(sources, num_batches)
        target_batches = np.array_split(targets, num_batches)
        
        transport_plan_batches = []
        for source_batch, target_batch in tqdm(zip(source_batches, target_batches), total=len(source_batches)):
            # Compute the cost matrix between the image and the Gaussian variables
            cost_matrix = ot.dist(source_batch, target_batch, metric='sqeuclidean')
            # Compute EMD
            a = np.ones(source_batch.shape[0]) / source_batch.shape[0]
            b = np.ones(target_batch.shape[0]) / target_batch.shape[0]
        
            transport_plan_batches.append(ot.emd([],[], cost_matrix, numItermax=10000000, numThreads=os.cpu_count()))
        

        return transport_plan_batches,source_batches,target_batches
    
    transport_plan_batches,source_batches,target_batches = batch(image_flat, gaussian_flat)
    gaussianized_image = np.zeros_like(image_flat)

    # Map each batch individually
    start_idx = 0
    for P, target_batch in zip(transport_plan_batches, target_batches):
        end_idx = start_idx + P.shape[0]
        for i in range(P.shape[0]):
            # Find the pixel in the target batch with the highest transport probability
            gaussianized_image[start_idx + i] = target_batch[P[i].argmax()]
        start_idx = end_idx

    gaussianized_image = gaussianized_image.reshape((height, width, channels))


    return gaussianized_image


if __name__ == '__main__':
    # input image path
    input_image_path = 'data/noise/fire_256.png'
    gaussianized_img = gaussianize_image(input_image_path)

    # just keep the file name + _g, and put it under output folder
    output_path = os.path.join('output', os.path.basename(input_image_path).split('.')[0] + '_batched_g.png')
    io.imsave(output_path, (gaussianized_img*255).astype('uint8'))




