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
#Direct approach. A direct approach for inverting the transformation could be to use the optimal
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

# the lut_size is set to 24 
# as it would take too much space to fit in memory during calculation
# it will be updated to a batched version once it's correctly implemented.
def create_lut(ref_image, lut_size=24, isSave=True):
    """
    Create LUT with proper range mapping
    """
    # Calculate theoretical U range
    u_min = 0.5 + 0.5 * erf(-0.5/(6*np.sqrt(2)))  # ≈ 0.467
    u_max = 0.5 + 0.5 * erf(0.5/(6*np.sqrt(2)))   # ≈ 0.533
    u_range = u_max - u_min
    
    # Create uniform grid mapped to expected U range
    u = np.linspace(0, 1, lut_size)  # Use full [0,1] range
    # Map [0,1] to [u_min, u_max]
    u = u * u_range + u_min
    
    grid = np.stack(np.meshgrid(u, u, u, indexing='ij'), -1)
    grid = grid.reshape(-1, 3)
    N = lut_size

    # Transform U coordinates to Gaussian space
    G = 0.5 + 6 * np.sqrt(2) * erfinv(2 * grid - 1)
    G = np.clip(G, 0, 1-1e-6)

    # Sample reference colors
    ref_samples = ref_image.reshape(-1, 3)
    if len(ref_samples) < len(grid):
        indices = np.random.choice(len(ref_samples), len(grid), replace=True)
        ref_samples = ref_samples[indices]
    else:
        indices = np.random.choice(len(ref_samples), len(grid), replace=False)
        ref_samples = ref_samples[indices]
    
    # Create transport mapping
    cost_matrix = ot.dist(grid, ref_samples)
    P = ot.emd(np.ones(len(grid))/len(grid), 
               np.ones(len(ref_samples))/len(ref_samples), 
               cost_matrix, 
               numItermax=10000000, 
               numThreads=os.cpu_count())
    
    # Create LUT
    LUT = np.zeros((N, N, N, 3))
    for i in range(N):
        for j in range(N):
            for k in range(N):
                idx = i * N * N + j * N + k
                LUT[i, j, k] = ref_samples[P[idx].argmax()]

    if isSave:
        np.save('lut.npy', LUT)

    return LUT

def apply_inverse_mapping(gaussian_image, LUT):
    """
    Apply inverse mapping with trilinear interpolation
    """
    N = LUT.shape[0]
    width, height, _ = gaussian_image.shape
    
    # Calculate U range
    u_min = 0.5 + 0.5 * erf(-0.5/(6*np.sqrt(2)))
    u_max = 0.5 + 0.5 * erf(0.5/(6*np.sqrt(2)))
    u_range = u_max - u_min
    
    # Reshape input for processing
    gaussian_image = gaussian_image.reshape(-1, 3)

    # Transform Gaussian to U space
    U = 0.5 + 0.5 * erf((gaussian_image-0.5) / (6 * np.sqrt(2)))
    U = np.clip(U, u_min, u_max)
    
    # Map from U range to [0,1] for LUT coordinates
    U = (U - u_min) / u_range
    U = U * (N-1)  # Scale to LUT size
    
    # Get integer coordinates and fractions for interpolation
    i0 = np.floor(U).astype(int)
    i1 = np.minimum(i0 + 1, N-1)
    alpha = U - i0
    
    # Initialize result array
    result = np.zeros_like(gaussian_image)
    
    # Apply trilinear interpolation
    for i in range(len(U)):
        x0, y0, z0 = i0[i]
        x1, y1, z1 = i1[i]
        fx, fy, fz = alpha[i]
        
        # Get the 8 surrounding colors
        c000 = LUT[x0, y0, z0]
        c001 = LUT[x0, y0, z1]
        c010 = LUT[x0, y1, z0]
        c011 = LUT[x0, y1, z1]
        c100 = LUT[x1, y0, z0]
        c101 = LUT[x1, y0, z1]
        c110 = LUT[x1, y1, z0]
        c111 = LUT[x1, y1, z1]
        
        # Trilinear interpolation
        result[i] = (
            c000 * (1-fx) * (1-fy) * (1-fz) +
            c001 * (1-fx) * (1-fy) * fz +
            c010 * (1-fx) * fy * (1-fz) +
            c011 * (1-fx) * fy * fz +
            c100 * fx * (1-fy) * (1-fz) +
            c101 * fx * (1-fy) * fz +
            c110 * fx * fy * (1-fz) +
            c111 * fx * fy * fz
        )
    
    return result.reshape(width, height, 3)


def process_texture(gaussian_image_path, reference_image_path):
    # Load the input image
    gaussian_image = load_image(gaussian_image_path)
    # Load the reference image
    reference_image = load_image(reference_image_path)

    # Create LUT mapping
    LUT = create_lut(reference_image, 24)
    
    # Apply mapping with interpolation
    result = apply_inverse_mapping(gaussian_image, LUT)
    
    return np.clip(result, 0, 1)


if __name__ == '__main__':
    gaussian_image_path = './blended/wood_blended.png'
    reference_image_path = './data/noise/wood_256.png'

    gaussian_image = load_image(gaussian_image_path)
    reference_image = load_image(reference_image_path)

    #sampling from reference image
    reference_image = reference_image.reshape(-1, 3)
    indices = np.random.choice(reference_image.shape[0], 1600, replace=False)
    reference_image = reference_image[indices]
    # resize
    reference_image = reference_image.reshape(40, 40, 3)

    result = optimal_transport(gaussian_image, reference_image, allow_diff_dimensions=True, sample_from_dest=True, batch_size=1000)
    # result = process_texture(gaussian_image_path, reference_image_path)
    io.imsave('mapped/wood_ot_mapped.png', (result*255).astype('uint8'))