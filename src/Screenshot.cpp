#pragma once
#include "NoiseSynth.h"
#include <opencv2/opencv.hpp>

// Captures the current OpenGL framebuffer and saves it to a file
bool NoiseSynth::saveScreenshot(const std::string& filename, int width, int height) {
    if(!hideGUI){
        hideGUI = true;
        return false;
    }
    // Allocate memory for the pixel data (RGBA format)
    std::vector<unsigned char> pixels(width * height * 4);
    
    // Read pixels from the framebuffer
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Create OpenCV Mat (note: OpenGL gives RGBA, but we'll convert to BGR for OpenCV)
    cv::Mat img(height, width, CV_8UC4, pixels.data());
    
    // Flip vertically because OpenGL and OpenCV have different coordinate systems
    cv::flip(img, img, 0);
    
    // Convert RGBA to BGR format
    cv::Mat bgr;
    cv::cvtColor(img, bgr, cv::COLOR_RGBA2BGR);
    
    // Save image
    return cv::imwrite(filename, bgr);
};