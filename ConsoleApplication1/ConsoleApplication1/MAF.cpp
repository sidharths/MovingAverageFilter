// MAF.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MAF.h"


int main()
{
	//FilterStep1 will work on only numbers and use a width=3 filter. The inputs are same as in example provided

	MovingAverageFilter FilterStep1(3); // calling the construtor with filter width=3

	double a[11] = { 5,20,45,27,50 };  //input array
	std::cout << "Background removal for only numbers" << std::endl;
	for (auto i = 0; i < 5; ++i) {
		std::cout << std::fixed;
		std::cout.precision(2);
		std::cout.width(9);
		std::cout << FilterStep1.BackgroundSubFilter(a[i]); //The core call to remove the background
	}
	std::cout << std::endl << std::endl << std::endl;


	std::vector< std::vector<double> > traces = { { 20,25,70,35 },
	{ 22,26,75,32 },
	{ 25,23,74,32 },
	{ 21,25,68,34 },
	{ 18,22,71,38 },
	{ 14,20,70,42 }
	};   // input for both step 2 and step 3
	MovingAverageFilter FilterStep2(5, 4); // calling the construtor with filter width=3, tracelength=4, and not using weighted mean by default

	std::cout << "Background removal for Traces" << std::endl;
	for (auto i = 0; i < 6; ++i) {
		std::vector<double> ret = FilterStep2.BackgroundSubFilter(traces[i]);
		std::cout << "Trace " << i + 1 << " ->";
		for (auto j = 0; j < 4; ++j) {
			std::cout << std::fixed;
			std::cout.precision(2);
			std::cout.width(9);
			std::cout << ret[j] << "\t";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl << std::endl << std::endl;


	// calling the construtor with filter width=5, tracelength=4, and using weighted mean
	// The constructor also initializes the weights array based on the filterwidth
	MovingAverageFilter FilterStep3(5, 4, true);

	std::cout << "Background removal for Traces with weighted average" << std::endl;
	for (auto i = 0; i < 6; ++i) {
		std::vector<double> ret = FilterStep3.BackgroundSubFilter(traces[i]);
		std::cout << "Trace " << i + 1 << " ->";
		for (auto j = 0; j < 4; ++j) {
			std::cout << std::fixed;
			std::cout.precision(2);
			std::cout.width(9);
			std::cout << ret[j] << "\t";
		}
		std::cout << std::endl;
	}


	return 0;
}


/*
Initializes filterwidth
*/
MovingAverageFilter::MovingAverageFilter(size_t w)
{
	mFilterWidth = w;
}


/*
Initializes filterwidth, tracelength
*/
MovingAverageFilter::MovingAverageFilter(size_t w, size_t TraceLength) : mTraceLength(TraceLength), mFilterWidth(w)
{
	mRunningSumVector.resize(TraceLength, 0.0);
}


/*
Initializes filterwidth, tracelength and weights
*/
MovingAverageFilter::MovingAverageFilter(size_t w, size_t TraceLength, bool UseWeights) :MovingAverageFilter(w, TraceLength)
{
	if (UseWeights) {
		double base = 1.0 / w;
		//Initialize weigths
		for (size_t i = 0; i < w; ++i)
			mWeights.push_back(base + i*base);
	}
}

MovingAverageFilter::~MovingAverageFilter()
{

}

/*
Initializes filterwidth
*/
double MovingAverageFilter::RunningAverage(double w)
{
	// if the deque is smaller than filter width keep updating runningsum
	if (mWindow.size() < mFilterWidth) {
		mRunningSum += w;
	}
	else { // else subtract the front from runningsum and add the new value w. pop the front item in queue
		double& front = mWindow.front();
		mRunningSum -= front;
		mRunningSum += w;
		mWindow.pop_front(); //remove element from front
	}

	mWindow.push_back(w);

	return mRunningSum / mWindow.size();

}

std::vector<double>  MovingAverageFilter::RunningAverage(std::vector<double> w)
{

	//same as the number case, but now we have a tracelength size vector of runningsum
	//update the deque in same manner

	if (mWindowVector.size() < mFilterWidth) {
		int j = 0;
		for (auto i = w.begin(); i < w.end(); ++i) {
			mRunningSumVector[j] += *i;
			++j;
		}
	}
	else {
		std::vector<double>& front = mWindowVector.front();
		int j = 0;
		for (auto i = w.begin(); i < w.end(); ++i) {
			mRunningSumVector[j] -= front[j];
			mRunningSumVector[j] += *i;
			++j;
		}
		mWindowVector.pop_front(); //remove element from front
	}

	mWindowVector.push_back(w);
	std::vector<double> ret;
	for (auto i = mRunningSumVector.begin(); i < mRunningSumVector.end(); ++i) {
		ret.push_back((*i) / mWindowVector.size());
	}

	return ret;
}




std::vector<double>  MovingAverageFilter::RunningAverageWeighted(std::vector<double> w)
{
	std::vector<double> sum;
	sum.resize(mTraceLength, 0.0);

	int j = mFilterWidth - 2;
	//Here approach is totally different. We will straight away pop the front of deque if the deque is of filterwidth size.
	if (mWindowVector.size() == mFilterWidth)
		mWindowVector.pop_front(); //remove element from front


								   //iteratre through the deque in reverse and find the weighted sum
								   //notice we havent added the current vector 'w' yet. 
	for (auto i = mWindowVector.rbegin(); i != mWindowVector.rend(); ++i) {
		std::vector<double> trace = *i;
		for (size_t k = 0; k<mTraceLength; ++k)
			sum[k] += (mWeights[j] * trace[k]);
		j--;
	}

	// now add the current vector 'w'. It has weight 1 so add it as it is.
	for (size_t k = 0; k<mTraceLength; ++k)
		sum[k] += (w[k]);
	mWindowVector.push_back(w);

	//Based on the size of deque find the sum of weights used
	double SumWeights = 0.0;
	for (auto i = mFilterWidth - 1, j = mWindowVector.size(); j>0; --j, --i)
		SumWeights += mWeights[i];

	//prepare the return vector and it will contain the running weighted average
	std::vector<double> ret;
	for (size_t i = 0; i <mTraceLength; ++i) {
		ret.push_back(sum[i] / SumWeights);
	}
	return ret;
}


/*
Calls the running average routine and subtracts the running average (background) from the current trace value.

*/


double MovingAverageFilter::BackgroundSubFilter(double current)
{
	double  rAvg = RunningAverage(current);
	return  current - rAvg;
}


std::vector<double>	MovingAverageFilter::BackgroundSubFilter(std::vector<double> current)
{
	std::vector<double> rAvg;

	//if the mWeights array was populated then call the routine for handling weighted traces, otherwise call the routine optimized to use running sum.
	if (mWeights.empty())
		rAvg = RunningAverage(current);
	else
		rAvg = RunningAverageWeighted(current);

	std::vector<double> ret;

	for (size_t i = 0; i < current.size(); ++i)
		ret.push_back(current[i] - rAvg[i]);

	return ret;
}