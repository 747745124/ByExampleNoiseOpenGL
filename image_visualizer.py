import numpy as np
import matplotlib.pyplot as plt
from skimage import io
from skimage.exposure import histogram

def plot_rgb_histogram(image_path, bins=256, figure_size=(10, 6)):
    """
    Plot RGB histogram for an input image using skimage.
    
    Parameters:
    image_path (str): Path to the image file
    bins (int): Number of bins for the histogram
    figure_size (tuple): Size of the output figure (width, height)
    """
    # Read image using skimage
    img = io.imread(image_path)
    
    # Create figure and axis
    plt.figure(figsize=figure_size)
    
    # Plot histograms for each channel
    colors = ('red', 'green', 'blue')
    for i, color in enumerate(colors):
        # Calculate histogram using skimage's histogram function
        hist, bin_centers = histogram(img[:,:,i], nbins=bins, normalize=True)
        
        # Apply light smoothing using moving average
        smoothed_hist = np.convolve(hist, np.ones(5)/5, mode='valid')
        smoothed_bins = bin_centers[2:-2]  # Adjust bins to match smoothed histogram length
        
        plt.plot(smoothed_bins, smoothed_hist, color=color, alpha=0.7, 
                linewidth=2, label=color.upper())
    
    # Customize plot
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Normalized Frequency')
    plt.title('RGB Color Histogram')
    plt.legend()
    plt.grid(True, alpha=0.2)
    
    # Remove top and right spines
    plt.gca().spines['top'].set_visible(False)
    plt.gca().spines['right'].set_visible(False)
    
    plt.show()

def plot_rgb_histograms_comparison(image1_path, image2_path, bins=256, figure_size=(15, 5)):
    """
    Plot RGB histograms for two images side by side for comparison.
    
    Parameters:
    image1_path (str): Path to the first image
    image2_path (str): Path to the second image
    bins (int): Number of bins for the histogram
    figure_size (tuple): Size of the output figure (width, height)
    """
    # Read both images
    img1 = io.imread(image1_path)
    img2 = io.imread(image2_path)
    
    # Create subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=figure_size)
    
    # Plot histograms for each image
    for ax, img, title in zip([ax1, ax2], [img1, img2], ['Image 1', 'Image 2']):
        colors = ('red', 'green', 'blue')
        for i, color in enumerate(colors):
            hist, bin_centers = histogram(img[:,:,i], nbins=bins, normalize=True)
            
            # Apply smoothing
            smoothed_hist = np.convolve(hist, np.ones(5)/5, mode='valid')
            smoothed_bins = bin_centers[2:-2]
            
            ax.plot(smoothed_bins, smoothed_hist, color=color, alpha=0.7, 
                   linewidth=2, label=color.upper())
        
        ax.set_xlabel('Pixel Intensity')
        ax.set_ylabel('Normalized Frequency')
        ax.set_title(f'RGB Histogram - {title}')
        ax.legend()
        ax.grid(True, alpha=0.2)
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)
    
    plt.tight_layout()
    plt.show()

from skimage.exposure import histogram
from skimage.util import compare_images

def plot_image_difference(image1_path, image2_path, figure_size=(15, 10)):
    """
    Visualize the difference between two images, showing:
    1. Original images
    2. Difference image
    3. RGB histograms of the difference
    
    Parameters:
    image1_path (str): Path to the first image
    image2_path (str): Path to the second image
    figure_size (tuple): Size of the output figure (width, height)
    """
    # Read images
    img1 = io.imread(image1_path)
    img2 = io.imread(image2_path)
    
    # Ensure images have same shape
    if img1.shape != img2.shape:
        raise ValueError("Images must have the same dimensions")
    
    # Calculate difference
    diff = compare_images(img1, img2, method='diff')
    
    # Create figure with subplots
    fig = plt.figure(figsize=figure_size)
    gs = fig.add_gridspec(2, 3)
    
    # Plot original images and difference
    ax1 = fig.add_subplot(gs[0, 0])
    ax2 = fig.add_subplot(gs[0, 1])
    ax3 = fig.add_subplot(gs[0, 2])
    ax4 = fig.add_subplot(gs[1, :])
    
    # Display images
    ax1.imshow(img1)
    ax1.set_title('Image 1')
    ax1.axis('off')
    
    ax2.imshow(img2)
    ax2.set_title('Image 2')
    ax2.axis('off')
    
    # Display difference image
    diff_img = ax3.imshow(diff, cmap='RdBu_r')
    ax3.set_title('Difference')
    ax3.axis('off')
    plt.colorbar(diff_img, ax=ax3)
    
    # Plot histogram of differences for each channel
    colors = ('red', 'green', 'blue')
    for i, color in enumerate(colors):
        # Calculate histogram of difference
        hist, bin_centers = histogram(diff[:,:,i], nbins=256, normalize=True)
        
        # Apply smoothing
        smoothed_hist = np.convolve(hist, np.ones(5)/5, mode='valid')
        smoothed_bins = bin_centers[2:-2]
        
        ax4.plot(smoothed_bins, smoothed_hist, color=color, alpha=0.7,
                linewidth=2, label=color.upper())
    
    # Customize histogram plot
    ax4.set_xlabel('Difference Intensity')
    ax4.set_ylabel('Normalized Frequency')
    ax4.set_title('RGB Histogram of Differences')
    ax4.legend()
    ax4.grid(True, alpha=0.2)
    ax4.spines['top'].set_visible(False)
    ax4.spines['right'].set_visible(False)
    
    plt.tight_layout()
    plt.show()

def analyze_image_differences(image1_path, image2_path):
    """
    Analyze and print statistical measures of differences between two images.
    
    Parameters:
    image1_path (str): Path to the first image
    image2_path (str): Path to the second image
    """
    # Read images
    img1 = io.imread(image1_path)
    img2 = io.imread(image2_path)
    
    # Calculate difference
    diff = compare_images(img1, img2, method='diff')
    
    # Calculate statistics for each channel
    stats = {}
    for i, channel in enumerate(['Red', 'Green', 'Blue']):
        channel_diff = diff[:,:,i]
        stats[channel] = {
            'mean': np.mean(channel_diff),
            'std': np.std(channel_diff),
            'max': np.max(channel_diff),
            'min': np.min(channel_diff),
            'median': np.median(channel_diff)
        }
    
    # Print statistics
    print("Image Difference Statistics:")
    print("-" * 50)
    for channel, measures in stats.items():
        print(f"\n{channel} Channel:")
        for measure, value in measures.items():
            print(f"{measure.capitalize():>10}: {value:.4f}")

# Example usage
def main():
    # Example with sample image paths
    # image_path = "path_to_your_image.jpg"
    # plot_rgb_histogram(image_path)
    
    # For comparison of two images
    image1_path = "./output/wood_256_g_10m.png"
    image2_path = "./output/wood_256_g_builtin.png"
    # plot_rgb_histograms_comparison(image1_path, image2_path)
    plot_image_difference(image1_path, image2_path)

if __name__ == "__main__":
    main()