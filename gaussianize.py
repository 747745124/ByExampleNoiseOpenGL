import numpy as np
import ot  # Python Optimal Transport library
from scipy.stats import norm
from skimage import io, img_as_float
from scipy.special import erfinv
import os
from tqdm import tqdm
from image_scaler import load_image

# this gaussianize step requires batch processing,
# as the cost matrix is too large to fit in memory
# computing cost matrix on a scale of 256x256x3 will result in more than 
# 32GB of memory usage with POT library
# so we need to compute it in batches

# A batched implementation of the optimal transport, to avoid OOM
def batch(sources, targets, batch_size = 10000):
    # Calculating the transport plan in batches, requires flatten images
    num_source = sources.shape[0]
    num_target = targets.shape[0]
    
    print(f"Computing optimal transport in batches with batch size {batch_size}...")
    # Split data into batches
    num_batches = max(num_source // batch_size + 1, num_target // batch_size + 1)
    source_batches = np.array_split(sources, num_batches)
    target_batches = np.array_split(targets, num_batches)
    
    transport_plan_batches = []
    for source_batch, target_batch in tqdm(zip(source_batches, target_batches), total=len(source_batches)):
        # Compute the cost matrix between the image and the Gaussian variables
        cost_matrix = ot.dist(source_batch, target_batch, metric='sqeuclidean')

        transport_plan_batches.append(ot.emd([],[], cost_matrix, numItermax=10000000, numThreads=os.cpu_count()))
    
    print("Optimal transport computed.")
    return transport_plan_batches,source_batches,target_batches

def create_random_gaussian_image(shape, mean, variance, useBuiltin=True, useClipping=True):
    height, width, _ = shape
    sigma = np.sqrt(variance)

    if useBuiltin:
        gaussian_variables = np.random.normal(mean, sigma, (height, width, 3))
    else:
        gaussian_variables = np.zeros((height, width, 3))
        for channel in range(3):
            # Generate uniform random numbers
            uniform_random = np.random.rand(height, width)
            # Convert uniform to Gaussian using inverse CDF (ppf in scipy)
            gaussian_variables[:, :, channel] = mean + sigma * np.sqrt(2) * erfinv(2 * uniform_random - 1)
    
    if useClipping:
        gaussian_variables = np.clip(gaussian_variables, mean - 4*sigma, mean + 4*sigma)

    return gaussian_variables

def optimal_transport(src_image, dest_image, allow_diff_dimensions=False, sample_from_dest=False):

    src_image_flat = src_image.reshape((src_image.shape[0] * src_image.shape[1], src_image.shape[2])).astype(np.float64)
    dest_image_flat = dest_image.reshape((dest_image.shape[0] * dest_image.shape[1], dest_image.shape[2])).astype(np.float64)

    # if different dimensions, sampling is required
    # Note: this is not the best way to handle different dimensions, as it will lose local features.
    if src_image_flat.shape[0] != dest_image_flat.shape[0]:
        if not allow_diff_dimensions:
            raise ValueError("The images must have the same number of pixels.")
        else:
            # when the number of pixels is different, we either sample from the source or the destination
            if sample_from_dest:
                if src_image_flat.shape[0] > dest_image_flat.shape[0]:
                    indices = np.random.choice(dest_image_flat.shape[0], src_image_flat.shape[0], replace=True)
                else:
                    indices = np.random.choice(dest_image_flat.shape[0], src_image_flat.shape[0], replace=False)
                
                dest_image_flat = dest_image_flat[indices]

            else:
                if src_image_flat.shape[0] > dest_image_flat.shape[0]:
                    indices = np.random.choice(src_image_flat.shape[0], dest_image_flat.shape[0], replace=False)
                else:
                    indices = np.random.choice(src_image_flat.shape[0], dest_image_flat.shape[0], replace=True)
            
                src_image_flat = src_image_flat[indices]

    # Compute the transport plan in batches
    transport_plan_batches, source_batches, target_batches = batch(src_image_flat, dest_image_flat)
    gaussianized_image = np.zeros_like(src_image_flat)

    # Map each batch individually
    print("Mapping each batch individually...")
    start_idx = 0
    for P, target_batch in zip(transport_plan_batches, target_batches):
        end_idx = start_idx + P.shape[0]
        for i in range(P.shape[0]):
            # Find the pixel in the target batch with the highest transport probability
            gaussianized_image[start_idx + i] = target_batch[P[i].argmax()]
        start_idx = end_idx

    print("Mapping complete.")

    if sample_from_dest:
        gaussianized_image = gaussianized_image.reshape((src_image.shape[0],src_image.shape[1], src_image.shape[2]))
    else:
        gaussianized_image = gaussianized_image.reshape((dest_image.shape[0], dest_image.shape[1], dest_image.shape[2]))

    return gaussianized_image

    
def gaussianize_image(input_image_path, mean=0.5, variance=1/36):
    image = load_image(input_image_path)
    gaussianize_variables = create_random_gaussian_image(image.shape, mean, variance)

    assert image.shape == gaussianize_variables.shape, "The images must have the same dimensions."
    gaussianized_image = optimal_transport(image, gaussianize_variables)

    return gaussianized_image


if __name__ == '__main__':
    # Gaussianize an texture image here
    # input image path
    input_image_path = 'data/noise/fire_128.png'
    result = gaussianize_image(input_image_path)

    # just keep the file name + _g, and put it under output folder
    output_path = os.path.join('output', os.path.basename(input_image_path).split('.')[0] + '_g.png')
    io.imsave(output_path, (result*255).astype('uint8'))



