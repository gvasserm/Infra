#pragma once

#include <cstring>
#include <vector>

class cameraIntrinsic
{
    public:
        std::vector<float> focalLength;     // Camera focal length.
        std::vector<float> principalPoint;  // Principal point coordinates.
        std::vector<float> distortionCoef;  // Distortion coefficients. 10 - arbitrary number.
        int distortionCoefLength;
        float skewCoef;           // Skew coefficient.
        std::vector<float> fov;
        int frameHeight;
        int frameWidth;

        void reset()
        {
            focalLength = std::vector<float>(2,0);
            principalPoint = std::vector<float>(2,0);
            distortionCoef = std::vector<float>(10,0);
            fov = std::vector<float>(2,0);
            skewCoef = 0.0f;
            distortionCoefLength = 0;
            frameHeight=0;
            frameWidth=0;
        };

        cameraIntrinsic()
        {
            reset();
	    };
};