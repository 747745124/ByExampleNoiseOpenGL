import numpy as np
import matplotlib.pyplot as plt
from skimage import io
from skimage.exposure import histogram
from image_scaler import load_image

def visualize_lut_mapping(LUT):
    N = LUT.shape[0]
    
    # Create a figure with multiple slices
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    
    # Show different z-slices (fixed k values)
    k_values = [0, N//2, N-1]  # Beginning, middle, end slices
    for idx, k in enumerate(k_values):
        # Top row: Show the actual RGB colors
        axes[0, idx].imshow(LUT[:, :, k])
        axes[0, idx].set_title(f'RGB colors at k={k}')
        axes[0, idx].axis('off')
        
        # Bottom row: Show the color distribution
        r = LUT[:, :, k, 0].flatten()
        g = LUT[:, :, k, 1].flatten()
        b = LUT[:, :, k, 2].flatten()
        
        axes[1, idx].scatter(r, g, c=LUT[:, :, k].reshape(-1, 3), alpha=0.5)
        axes[1, idx].set_xlabel('Red')
        axes[1, idx].set_ylabel('Green')
        axes[1, idx].set_title(f'R-G distribution at k={k}')
    
    plt.tight_layout()
    plt.show()

    # Show statistics
    print("LUT Statistics:")
    print(f"Shape: {LUT.shape}")
    print("\nValue ranges:")
    print(f"Red:   [{LUT[...,0].min():.3f}, {LUT[...,0].max():.3f}]")
    print(f"Green: [{LUT[...,1].min():.3f}, {LUT[...,1].max():.3f}]")
    print(f"Blue:  [{LUT[...,2].min():.3f}, {LUT[...,2].max():.3f}]")
    
    # Test some lookups
    print("\nSample lookups:")
    print(f"Corner (0,0,0): {LUT[0,0,0]}")
    print(f"Center: {LUT[N//2,N//2,N//2]}")
    print(f"Corner (N-1,N-1,N-1): {LUT[N-1,N-1,N-1]}")

def analyze_color_coverage(LUT, original_texture):
    # Flatten both to Nx3 arrays of colors
    lut_colors = LUT.reshape(-1, 3)
    orig_colors = original_texture.reshape(-1, 3)
    
    # Plot 3D color distributions
    fig = plt.figure(figsize=(15, 5))
    
    # Original texture colors
    ax1 = fig.add_subplot(131, projection='3d')
    ax1.scatter(orig_colors[:,0], orig_colors[:,1], orig_colors[:,2], 
                c=orig_colors, marker='.', alpha=0.1)
    ax1.set_title('Original Texture Colors')
    ax1.set_xlabel('R')
    ax1.set_ylabel('G')
    ax1.set_zlabel('B')
    
    # LUT colors
    ax2 = fig.add_subplot(132, projection='3d')
    ax2.scatter(lut_colors[:,0], lut_colors[:,1], lut_colors[:,2], 
                c=lut_colors, marker='.', alpha=0.1)
    ax2.set_title('LUT Colors')
    ax2.set_xlabel('R')
    ax2.set_ylabel('G')
    ax2.set_zlabel('B')
    
    # 2D projection (R-G plane)
    ax3 = fig.add_subplot(133)
    ax3.scatter(orig_colors[:,0], orig_colors[:,1], c='blue', 
                label='Original', alpha=0.1)
    ax3.scatter(lut_colors[:,0], lut_colors[:,1], c='red', 
                label='LUT', alpha=0.1)
    ax3.set_title('R-G Projection')
    ax3.set_xlabel('R')
    ax3.set_ylabel('G')
    ax3.legend()
    
    plt.tight_layout()
    plt.show()
    
    # Print statistics
    print("Color Range Statistics:")
    print("\nOriginal Texture:")
    print(f"R: [{orig_colors[:,0].min():.3f}, {orig_colors[:,0].max():.3f}]")
    print(f"G: [{orig_colors[:,1].min():.3f}, {orig_colors[:,1].max():.3f}]")
    print(f"B: [{orig_colors[:,2].min():.3f}, {orig_colors[:,2].max():.3f}]")
    
    print("\nLUT:")
    print(f"R: [{lut_colors[:,0].min():.3f}, {lut_colors[:,0].max():.3f}]")
    print(f"G: [{lut_colors[:,1].min():.3f}, {lut_colors[:,1].max():.3f}]")
    print(f"B: [{lut_colors[:,2].min():.3f}, {lut_colors[:,2].max():.3f}]")
    
    # Calculate coverage metrics
    def calculate_coverage(orig, lut, bins=50):
        # Create 3D histograms
        orig_hist, edges = np.histogramdd(orig, bins=bins)
        lut_hist, _ = np.histogramdd(lut, bins=edges)
        
        # Calculate coverage
        covered_bins = np.sum((orig_hist > 0) & (lut_hist > 0))
        total_used_bins = np.sum(orig_hist > 0)
        coverage = covered_bins / total_used_bins if total_used_bins > 0 else 0
        
        return coverage * 100

    coverage = calculate_coverage(orig_colors, lut_colors)
    print(f"\nColor Space Coverage: {coverage:.1f}%")


if __name__ == "__main__":
    LUT = np.load('lut_improved.npy')
    visualize_lut_mapping(LUT)
    original_texture = load_image('./data/noise/fire_256.png')
    analyze_color_coverage(LUT, original_texture)
