#pragma once  
#include <deque>  
#include <vector>  
#include <iostream>  

class MovingAverageFilter
{
	size_t								mFilterWidth;
	double								mRunningSum = 0.0;
	std::deque <double>					mWindow;

	size_t								mTraceLength;
	std::vector<double>					mRunningSumVector;
	std::deque <std::vector<double>>	mWindowVector;

	std::vector<double>					mWeights;

public:

	MovingAverageFilter(size_t w);
	MovingAverageFilter(size_t w, size_t TraceLength);
	MovingAverageFilter(size_t w, size_t TraceLength, bool UseWeights);
	~MovingAverageFilter();

	double               BackgroundSubFilter(double              current);
	std::vector<double>	 BackgroundSubFilter(std::vector<double> current);

	double				 RunningAverage(double              current);
	std::vector<double>  RunningAverage(std::vector<double> current);
	std::vector<double>  RunningAverageWeighted(std::vector<double> current);
};

