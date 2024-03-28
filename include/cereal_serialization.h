#ifndef CEREAL_CV
#define CEREAL_CV

#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <opencv2/core/core.hpp>

namespace cv
{
	template<class Archive, class T>
	void serialize(Archive & archive, cv::Mat_<T> & mat)
	{
		int rows, cols, type;
		bool continuous;

		rows = mat.rows;
		cols = mat.cols;
		type = mat.type();
		continuous = mat.isContinuous();

		/*
		if (rows == 0 || cols == 0) {			
			archive.finishNode();
			return;
		}
		*/
		
		archive & cereal::make_nvp("dims", mat.dims);
		archive & cereal::make_nvp("rows", rows);
		archive & cereal::make_nvp("cols", cols);
		archive & cereal::make_nvp("type", type);
		//ar & cereal::make_nvp("dims", mat.size());

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

#endif