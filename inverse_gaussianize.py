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
def create_lut(ref_image, lut_size=24):
    # Create uniform grid for LUT
    u = (np.arange(lut_size) + 0.5) / lut_size
    grid = np.stack(np.meshgrid(u, u, u, indexing='ij'), -1)
    grid = grid.reshape(-1, 3)
    N = lut_size

    # Each element in the grid is a Gaussian variable
    G = 0.5 + 6 * np.sqrt(2) * erfinv(2 * grid - 1)
    G = np.clip(G, -10, 10)

    ref_samples = ref_image.reshape(ref_image.shape[0]*ref_image.shape[1], 3)
    
    # Sample pixels from the ref image
    if len(ref_samples) > len(grid):
        indices = np.random.choice(len(ref_samples), len(grid), replace=True)
        ref_samples = ref_samples[indices]
    
    # Create transport mapping
    cost_matrix = ot.dist(grid, ref_samples)
    P = ot.emd([], [], cost_matrix, numItermax=10000000, numThreads=os.cpu_count())
    
    LUT = np.zeros((N, N, N, 3))
    for i in range(N):
        for j in range(N):
            for k in range(N):
                idx = i * N * N + j * N + k
                LUT[i, j, k] = ref_samples[P[idx].argmax()]

    # LUT = np.dot(P, ref_samples).reshape(N, N, N, 3)
    return LUT

def apply_inverse_mapping(gaussian_image, LUT):
    N = LUT.shape[0]
    
    # Transform Gaussian to uniform
    U = 0.5 + 0.5 * erf((gaussian_image-0.5) / (6 * np.sqrt(2)))
    U = np.clip(U, 1e-6, 1-1e-6)
    
    # Scale to LUT coordinates
    U = U * (N-1)
    
    # Get integer coordinates and interpolation weights
    i0 = np.floor(U[:,:,0]).astype(int)
    j0 = np.floor(U[:,:,1]).astype(int)
    k0 = np.floor(U[:,:,2]).astype(int)
    
    i1 = np.minimum(i0 + 1, N-1)
    j1 = np.minimum(j0 + 1, N-1)
    k1 = np.minimum(k0 + 1, N-1)
    
    # Interpolation weights
    wi = U[:,:,0] - i0
    wj = U[:,:,1] - j0
    wk = U[:,:,2] - k0
    
    # Trilinear interpolation
    c000 = LUT[i0, j0, k0]
    c001 = LUT[i0, j0, k1]
    c010 = LUT[i0, j1, k0]
    c011 = LUT[i0, j1, k1]
    c100 = LUT[i1, j0, k0]
    c101 = LUT[i1, j0, k1]
    c110 = LUT[i1, j1, k0]
    c111 = LUT[i1, j1, k1]
    
    c00 = c000 * (1-wi)[:,:,None] + c100 * wi[:,:,None]
    c01 = c001 * (1-wi)[:,:,None] + c101 * wi[:,:,None]
    c10 = c010 * (1-wi)[:,:,None] + c110 * wi[:,:,None]
    c11 = c011 * (1-wi)[:,:,None] + c111 * wi[:,:,None]
    
    c0 = c00 * (1-wj)[:,:,None] + c10 * wj[:,:,None]
    c1 = c01 * (1-wj)[:,:,None] + c11 * wj[:,:,None]
    
    return c0 * (1-wk)[:,:,None] + c1 * wk[:,:,None]

def process_texture(gaussian_image_path, reference_image_path):
    # Load the input image
    gaussian_image = load_image(gaussian_image_path)
    # Load the reference image
    reference_image = load_image(reference_image_path)

    # Create LUT mapping
    LUT = create_lut(reference_image)
    
    # Apply mapping with interpolation
    result = apply_inverse_mapping(gaussian_image, LUT)
    
    return np.clip(result, 0, 1)

if __name__ == '__main__':
    gaussian_image_path = './output/fire_256_g.png'
    reference_image_path = './data/noise/fire_128.png'
    # result = image_mapping(gaussian_image_path, reference_image_path, allow_diff_dimensions=True, sample_from_dest=True)
    result = process_texture(gaussian_image_path, reference_image_path)
    io.imsave('output/gaussian_mapped.png', (result*255).astype('uint8'))