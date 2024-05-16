#ifndef CEREAL_CV
#define CEREAL_CV

#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <opencv2/core/core.hpp>

#include "SampleClass.h"

namespace cv
{
	template<class Archive, class T>
	void serialize(Archive & archive, const cv::Mat_<T> & mat)
	{
		int rows, cols, type;
		bool continuous;

		rows = mat.rows;
		cols = mat.cols;
		type = mat.type();
		continuous = mat.isContinuous();
		
		archive & cereal::make_nvp("dims", mat.dims);
		archive & cereal::make_nvp("rows", rows);
		archive & cereal::make_nvp("cols", cols);
		archive & cereal::make_nvp("type", type);

		if (continuous) 
		{
			archive.setNextName("data");
			archive.startNode();
			archive.makeArray();
			
			for (auto && it : mat)
			{
				archive(it);
			}
			archive.finishNode();
		}
	}


	template<class Archive>
	void serialize(Archive & archive, cv::Mat & mat)
	{
		int rows, cols, type;
		bool continuous;

		if (Archive::is_saving::value)
		{
			rows = mat.rows;
			cols = mat.cols;
			type = mat.type();
			continuous = mat.isContinuous();
		}

		archive & cereal::make_nvp("dims", mat.dims);
		archive & cereal::make_nvp("rows", rows);
		archive & cereal::make_nvp("cols", cols);
		archive & cereal::make_nvp("type", type);

		archive.setNextName("data");
		archive.startNode();
		archive.makeArray();
		
		for (int i = 0; i < rows*cols; i++)
		{
			archive(mat.at<double>(i));
		}
		archive.finishNode();
	}



	template<class Archive>
	void serialize(Archive & archive, cv::DMatch & o)
	{
		archive(CEREAL_NVP(o.queryIdx),
			CEREAL_NVP(o.trainIdx),
			CEREAL_NVP(o.imgIdx),
			CEREAL_NVP(o.distance)
		);
	}
	
	template<class Archive>
	void serialize(Archive & archive, cv::Size &o)
	{
		archive(CEREAL_NVP(o.height),
			CEREAL_NVP(o.width)
		);
	}

	template<class Archive>
	void serialize(Archive & archive, cv::Point2f &o)
	{
		archive(CEREAL_NVP(o.x),
			CEREAL_NVP(o.y)
		);
	}

	template<class Archive>
	void serialize(Archive & archive, cv::KeyPoint &o)
	{
		archive(CEREAL_NVP(o.pt),
			CEREAL_NVP(o.angle),
			CEREAL_NVP(o.size),
			CEREAL_NVP(o.octave)
		);
	}
} // namespace cv

template<class Archive>
void serialize(Archive & archive, cameraIntrinsic & o)
{
	archive(CEREAL_NVP(o.focalLength),
		CEREAL_NVP(o.principalPoint),
		CEREAL_NVP(o.distortionCoef),
		CEREAL_NVP(o.distortionCoefLength),
		CEREAL_NVP(o.skewCoef),
		CEREAL_NVP(o.fov),
		CEREAL_NVP(o.frameHeight),
		CEREAL_NVP(o.frameWidth)
	);
}

#endif