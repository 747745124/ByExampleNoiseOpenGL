#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <cmath>

// LUT resolution
const int RESOLUTION = 128;

////////////////////////////////
// Compute the Emu and Eavg LUT
// Write to image
////////////////////////////////

// generate a sample vector in the hemisphere
glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness)
{
    using namespace glm;

    float a = roughness * roughness;
    float phi = 2.0f * M_PI * Xi.x;
    float cos_theta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

    // spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sin_theta;
    H.y = sin(phi) * sin_theta;
    H.z = cos_theta;

    // tangent space to world space
    vec3 up = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 tangent = normalize(glm::cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sample_vec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sample_vec);
}

// importance sampling method works more efficiently
// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
float VanDerCorput(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0f;

    float upper = NdotV;
    float lower = NdotV * (1.0f - k) + k;

    return upper / lower;
}

float GeometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness)
{
    float NdotV = std::max(glm::dot(N, V), 0.0f);
    float NdotL = std::max(glm::dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
};

// used to generate low-discrepancy sequences to sample the hemisphere
glm::vec2 Hammersley(uint i, uint N)
{
    return glm::vec2(float(i) / float(N), VanDerCorput(i));
}

// integrate the BRDF over the hemisphere using GGX importance sampling
glm::vec3 IntegrateBRDF(float roughness, glm::vec3 V)
{
    using namespace glm;

    const uint sample_count = 1024u;
    float result = 0.0f;
    vec3 N(0.0, 0.0, 1.0);

    for (uint i = 0; i < sample_count; i++)
    {
        vec2 Xi = Hammersley(i, sample_count);
        vec3 H = ImportanceSampleGGX(Xi, vec3(0.0f, 0.0f, 1.0f), 0.0f);
        vec3 L = normalize(H * 2.0f * dot(V, H) - V);

        // discard samples that are not in the hemisphere
        if (L.z < 0.0f)
            continue;

        float NoL = std::max(L.z, 0.0f);
        float NoH = std::max(H.z, 0.0f);
        float VoH = std::max(dot(V, H), 0.0f);
        float NoV = std::max(dot(N, V), 0.0f);

        float G = GeometrySmith(N, V, L, roughness);
        float G_Vis = (G * VoH) / (NoH * NoV);

        if (!isnan(G_Vis))
            result += G_Vis;
    }

    result /= float(sample_count);
    return vec3(min(result, 1.0f));
};

glm::vec3 getEavg(glm::vec3 V, float roughness, glm::vec3 Ei)
{
    const uint sample_count = 1024u;

    glm::vec3 Eavg(0.0f);
    glm::vec3 N(0.0, 0.0, 1.0);

    for (int i = 0; i < sample_count; i++)
    {
        glm::vec2 Xi = Hammersley(i, sample_count);
        glm::vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        glm::vec3 L = normalize(H * 2.0f * dot(V, H) - V);

        float NoL = std::max(L.z, 0.0f);
        // cosine weighted average
        Eavg += Ei * NoL;
    }

    // coeff is 2
    Eavg /= float(sample_count);

    return 2.0f * Eavg;
}

void Gamma_Correction(cv::Mat &src_mat, float gamma = 2.2f)
{
    float inv_gamma = 1.0f / gamma;
    for (int i = 0; i < src_mat.rows; i++)
    {
        for (int j = 0; j < src_mat.cols; j++)
        {
            cv::Vec3f &pixel = src_mat.at<cv::Vec3f>(i, j);
            pixel[0] = powf(pixel[0] / 255.f, inv_gamma) * 255.f;
            pixel[1] = powf(pixel[1] / 255.f, inv_gamma) * 255.f;
            pixel[2] = powf(pixel[2] / 255.f, inv_gamma) * 255.f;
        }
    }

    return;
};
void ComputeLUT_Eavg(const std::string dest_path, cv::Mat &src_mat)
{
    using namespace glm;
    cv::Mat img(RESOLUTION, RESOLUTION, CV_32FC3);
    cv::Mat img_v(RESOLUTION, RESOLUTION, CV_32FC3);
    float step_size = 1.0 / RESOLUTION;
    Gamma_Correction(src_mat, 2.2f);

    for (int i = 0; i < RESOLUTION; i++)
    {
        // same rough shares these params
        vec3 Eavg(0.0);
        float roughness = step_size * (float(i) + 0.5f);

        for (int j = 0; j < RESOLUTION; j++)
        {
            float NoV = step_size * (float(j) + 0.5f);
            vec3 V = vec3(sqrtf(1.0f - NoV * NoV), 0.0f, NoV);
            // normalize back to 0 to 1
            cv::Vec3f Ei = src_mat.at<cv::Vec3f>(i, j);
            Eavg += getEavg(V, roughness, vec3(Ei[0] / 255.f, Ei[1] / 255.f, Ei[2] / 255.f));
        }

        for (int k = 0; k < RESOLUTION; k++)
            img.at<cv::Vec3f>(i, k) = cv::Vec3f(Eavg[0], Eavg[1], Eavg[2]);
    }

    cv::flip(img, img_v, 0);
    cv::imwrite(dest_path.c_str(), img_v);
};
// precompute the Emu and write it to an image
cv::Mat ComputeLUT_Emu(const std::string dest_path)
{
    using namespace glm;
    using namespace std;
    cv::Mat img(RESOLUTION, RESOLUTION, CV_32FC3);
    cv::Mat img_v(RESOLUTION, RESOLUTION, CV_32FC3);

    float step_size = 1.0 / RESOLUTION;
    for (int i = 0; i < RESOLUTION; i++)
    {
        for (int j = 0; j < RESOLUTION; j++)
        {
            float roughness = step_size * (float(i) + 0.5f);
            float NoV = step_size * (float(j) + 0.5f);

            vec3 V = vec3(sqrt(1.0f - NoV * NoV), 0.0f, NoV);
            vec3 irradiance = IntegrateBRDF(roughness, V);
            img.at<cv::Vec3f>(i, j) = 255.f * cv::Vec3f(irradiance.x, irradiance.y, irradiance.z);
        }
    }

    cv::flip(img, img_v, 0);
    cv::imwrite(dest_path.c_str(), img_v);
    return img;
};

// int main()
// {
//     auto img = ComputeLUT_Emu("./Emu_LUT.png");
//     ComputeLUT_Eavg("./Eavg_LUT.png", img);
//     return 0;
// }