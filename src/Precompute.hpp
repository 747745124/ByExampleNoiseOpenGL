#pragma once
#include "NoiseSynth.h"
#include "jacobi.h"
// Ref: https://eheitzresearch.wordpress.com/738-2/
#define GAUSSIAN_AVERAGE 0.5f // Expectation of the Gaussian distribution
#define GAUSSIAN_STD 0.16666f // Std of the Gaussian distribution

using glm::vec3;
using glm::vec2;

float erfinv(float x)
{
	float w, p;
	w = -log((1.0f - x) * (1.0f + x));
	if (w < 5.000000f)
	{
		w = w - 2.500000f;
		p = 2.81022636e-08f;
		p = 3.43273939e-07f + p * w;
		p = -3.5233877e-06f + p * w;
		p = -4.39150654e-06f + p * w;
		p = 0.00021858087f + p * w;
		p = -0.00125372503f + p * w;
		p = -0.00417768164f + p * w;
		p = 0.246640727f + p * w;
		p = 1.50140941f + p * w;
	}
	else
	{
		w = sqrt(w) - 3.000000f;
		p = -0.000200214257f;
		p = 0.000100950558f + p * w;
		p = 0.00134934322f + p * w;
		p = -0.00367342844f + p * w;
		p = 0.00573950773f + p * w;
		p = -0.0076224613f + p * w;
		p = 0.00943887047f + p * w;
		p = 1.00167406f + p * w;
		p = 2.83297682f + p * w;
	}
	return p * x;
}

float CDF(float x, float mu, float sigma)
{
	float U = 0.5f * (1 + erf((x-mu)/(sigma*sqrtf(2.0f))));
	return U;
}

float invCDF(float U, float mu, float sigma)
{
	float x = sigma*sqrtf(2.0f) * erfinv(2.0f*U-1.0f) + mu;
	return x;
}

void ComputeinvT(TextureDataFloat& input, TextureDataFloat& Tinv, int channel)
{
	// Sort pixels of example image
	std::vector<float> sortedInputValues;
	sortedInputValues.resize(input.width * input.height);
	for (int y = 0; y < input.height; y++)
	for (int x = 0; x < input.width; x++)
	{
		sortedInputValues[y * input.width + x] = input.GetPixel(x, y, channel);
	}

	sort(sortedInputValues.begin(), sortedInputValues.end());

	// Generate Tinv look-up table 
	for (int i = 0; i < Tinv.width; i++)
	{
		// Gaussian value in [0, 1]
		float G = (i + 0.5f) / (Tinv.width);
		// Quantile value 
		float U = CDF(G, GAUSSIAN_AVERAGE, GAUSSIAN_STD);
		// Find quantile in sorted pixel values
		int index = (int)floor(U * sortedInputValues.size());
		// Get input value 
		float I = sortedInputValues[index];
		// Store in LUT
		Tinv.SetPixel(i, 0, channel, I);
	}
}


void ComputeEigenVectors(TextureDataFloat& input, vec3 eigenVectors[3])
{
	// First and second order moments
	float R=0, G=0, B=0, RR=0, GG=0, BB=0, RG=0, RB=0, GB=0;
	for (int y = 0; y < input.height; y++)
	for (int x = 0; x < input.width; x++)
	{
		vec3 col = input.GetColorAt(x, y);
		R += col.x;
		G += col.y;
		B += col.z;
		RR += col.x * col.x;
		GG += col.y * col.y;
		BB += col.z * col.z;
		RG += col.x * col.y;
		RB += col.x * col.z;
		GB += col.y * col.z;
	}	
	R /= (float)(input.width * input.height);
	G /= (float)(input.width * input.height);
	B /= (float)(input.width * input.height);
	RR /= (float)(input.width * input.height);
	GG /= (float)(input.width * input.height);
	BB /= (float)(input.width * input.height);
	RG /= (float)(input.width * input.height);
	RB /= (float)(input.width * input.height);
	GB /= (float)(input.width * input.height);
	
	// Covariance matrix
	double covarMat[3][3];
	covarMat[0][0] = RR - R*R;
	covarMat[0][1] = RG - R*G;
	covarMat[0][2] = RB - R*B;
	covarMat[1][0] = RG - R*G;
	covarMat[1][1] = GG - G*G;
	covarMat[1][2] = GB - G*B;
	covarMat[2][0] = RB - R*B;
	covarMat[2][1] = GB - G*B;
	covarMat[2][2] = BB - B*B;

	// Find eigen values and vectors using Jacobi algorithm
	double eigenVectorsTemp[3][3];
	double eigenValuesTemp[3];
	ComputeEigenValuesAndVectors(covarMat, eigenVectorsTemp, eigenValuesTemp);

	// Set return values
	eigenVectors[0] = vec3((float)eigenVectorsTemp[0][0], (float)eigenVectorsTemp[1][0], (float)eigenVectorsTemp[2][0]);
	eigenVectors[1] = vec3((float)eigenVectorsTemp[0][1], (float)eigenVectorsTemp[1][1], (float)eigenVectorsTemp[2][1]);
	eigenVectors[2] = vec3((float)eigenVectorsTemp[0][2], (float)eigenVectorsTemp[1][2], (float)eigenVectorsTemp[2][2]);
}

// Main function of Section 1.4
void DecorrelateColorSpace(
 TextureDataFloat& input,			  // input: example image
 TextureDataFloat& input_decorrelated,// output: decorrelated input 
 vec3& colorSpaceVector1,			  // output: color space vector1 
 vec3& colorSpaceVector2,			  // output: color space vector2
 vec3& colorSpaceVector3,			  // output: color space vector3
 vec3& colorSpaceOrigin)			  // output: color space origin
{
	// Compute the eigenvectors of the histogram
	vec3 eigenvectors[3];
	ComputeEigenVectors(input, eigenvectors);

	// Rotate to eigenvector space and 
	for (int y = 0; y < input.height; y++)
	for (int x = 0; x < input.width; x++)
	for(int channel = 0 ; channel < 3 ; ++channel)
	{
		// Get current color
		vec3 color = input.GetColorAt(x, y);
		// Project on eigenvector 
		float new_channel_value = glm::dot(color,eigenvectors[channel]);
		// Store
		input_decorrelated.SetPixel(x, y, channel, new_channel_value);
	}

	// Compute ranges of the new color space
	vec2 colorSpaceRanges[3] = {vec2(FLT_MAX,FLT_MIN), vec2(FLT_MAX,FLT_MIN), vec2(FLT_MAX,FLT_MIN)};
	for (int y = 0; y < input.height; y++)
	for (int x = 0; x < input.width; x++)
	for(int channel = 0 ; channel < 3 ; ++channel)
	{
		colorSpaceRanges[channel].x = fmin(colorSpaceRanges[channel].x, input_decorrelated.GetPixel(x,y,channel));
		colorSpaceRanges[channel].y = fmax(colorSpaceRanges[channel].y, input_decorrelated.GetPixel(x,y,channel));
	}

	// Remap range to [0, 1]
	for (int y = 0; y < input.height; y++)
	for (int x = 0; x < input.width; x++)
	for(int channel = 0 ; channel < 3 ; ++channel)
	{
		// Get current value
		float value = input_decorrelated.GetPixel(x,y,channel);
		// Remap in [0, 1]
		float remapped_value = (value - colorSpaceRanges[channel].x) / (colorSpaceRanges[channel].y - colorSpaceRanges[channel].x);
		// Store
		input_decorrelated.SetPixel(x, y, channel, remapped_value);
	}

	// Compute color space origin and vectors scaled for the normalized range
	colorSpaceOrigin.x = colorSpaceRanges[0].x * eigenvectors[0].x + colorSpaceRanges[1].x * eigenvectors[1].x + colorSpaceRanges[2].x * eigenvectors[2].x;
	colorSpaceOrigin.y = colorSpaceRanges[0].x * eigenvectors[0].y + colorSpaceRanges[1].x * eigenvectors[1].y + colorSpaceRanges[2].x * eigenvectors[2].y;
	colorSpaceOrigin.z = colorSpaceRanges[0].x * eigenvectors[0].z + colorSpaceRanges[1].x * eigenvectors[1].z + colorSpaceRanges[2].x * eigenvectors[2].z;
	colorSpaceVector1.x = eigenvectors[0].x * (colorSpaceRanges[0].y - colorSpaceRanges[0].x);
	colorSpaceVector1.y = eigenvectors[0].y * (colorSpaceRanges[0].y - colorSpaceRanges[0].x);
	colorSpaceVector1.z = eigenvectors[0].z * (colorSpaceRanges[0].y - colorSpaceRanges[0].x);
	colorSpaceVector2.x = eigenvectors[1].x * (colorSpaceRanges[1].y - colorSpaceRanges[1].x);
	colorSpaceVector2.y = eigenvectors[1].y * (colorSpaceRanges[1].y - colorSpaceRanges[1].x);
	colorSpaceVector2.z = eigenvectors[1].z * (colorSpaceRanges[1].y - colorSpaceRanges[1].x);
	colorSpaceVector3.x = eigenvectors[2].x * (colorSpaceRanges[2].y - colorSpaceRanges[2].x);
	colorSpaceVector3.y = eigenvectors[2].y * (colorSpaceRanges[2].y - colorSpaceRanges[2].x);
	colorSpaceVector3.z = eigenvectors[2].z * (colorSpaceRanges[2].y - colorSpaceRanges[2].x);
}

// Compute average subpixel variance at a given LOD
float ComputeLODAverageSubpixelVariance(TextureDataFloat& image, int LOD, int channel)
{
	// Window width associated with
	int windowWidth = 1 << LOD;
	
	// Compute average variance in all the windows
	float average_window_variance = 0.0;

	// Loop over al the windows
	for(int window_y = 0 ; window_y < image.height ; window_y += windowWidth)
	for(int window_x = 0 ; window_x < image.width  ; window_x += windowWidth)
	{
		// Estimate variance of current window
		float v = 0.0f; 
		float v2 = 0.0f; 
		for(int y = 0 ; y < windowWidth ; y++)
		for(int x = 0 ; x < windowWidth ; x++)
		{
			float value = image.GetPixel(window_x + x, window_y + y, channel);
			v  += value;
			v2 += value*value;
		}
		v  /= (float)(windowWidth*windowWidth);
		v2 /= (float)(windowWidth*windowWidth);
		float window_variance = fmax(0.0f, v2 - v*v);
		
		// Update average
		average_window_variance += window_variance / (image.width*image.height/windowWidth/windowWidth);
	}
	
	return average_window_variance;
}

// Filter LUT by sampling a Gaussian N(mu, std�)
float FilterLUTValueAtx(TextureDataFloat& LUT, float x, float std, int channel)
{
	// Number of samples for filtering (heuristic: twice the LUT resolution)
	const int numberOfSamples = 2 * 128;

	// Filter
	float filtered_value = 0.0f;
	for (int sample = 0; sample < numberOfSamples; sample++)
	{
		// Quantile used to sample the Gaussian
		float U = (sample + 0.5f) / numberOfSamples;
		// Sample the Gaussian 
		float sample_x = invCDF(U, x, std);
		// Find sample texel in LUT (the LUT covers the domain [0, 1])
		int sample_texel = fmax(0, fmin(128 - 1, (int)floor(sample_x * 128)));
		// Fetch LUT at level 0
		float sample_value = LUT.GetPixel(sample_texel, 0, channel);
		// Accumulate
		filtered_value += sample_value;
	}
	// Normalize and return
	filtered_value /= (float) numberOfSamples;
	return filtered_value;
}

// Main function of section 1.5
void PrefilterLUT(TextureDataFloat& image_T_Input, TextureDataFloat& LUT_Tinv, int channel)
{
	// Compute number of prefiltered levels and resize LUT
	LUT_Tinv.height = (int)(log((float)image_T_Input.width)/log(2.0f));
	LUT_Tinv.data.resize(3 * LUT_Tinv.width * LUT_Tinv.height);
	
	// Prefilter 
	for(int LOD = 1 ; LOD < LUT_Tinv.height ; LOD++)
	{
		// Compute subpixel variance at LOD 
		float window_variance = ComputeLODAverageSubpixelVariance(image_T_Input, LOD, channel);
		float window_std = sqrtf(window_variance);

		// Prefilter LUT with Gaussian kernel of this variance
		for (int i = 0; i < LUT_Tinv.width; i++)
		{
			// Texel position in [0, 1]
			float x_texel = (i+0.5f) / LUT_Tinv.width;
			// Filter look-up table around this position with Gaussian kernel
			float filteredValue = FilterLUTValueAtx(LUT_Tinv, x_texel, window_std, channel);
			// Store filtered value
			LUT_Tinv.SetPixel(i, LOD, channel, filteredValue);
		}
	}
}