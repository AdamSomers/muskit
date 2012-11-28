#ifndef WINDOWFUNCDISPLAY_H
#define WINDOWFUNCDISPLAY_H

/// \brief WindowFunction
/// Class to maintain window function tables
// and provide a simple "getNextSample" interface.
// This comes at a price (per-sample function calls)

// speedups:
// - provide references to the tables and make the caller deal with indexing
// - give the caller data on a per-block basis
//     - copy table portions
//     - give pointer into table? (still need to wrap..)
// - enforce power of 2 table size and use mask to wrap index

class WindowFunction
{
public: 
	WindowFunction(unsigned int size);
	~WindowFunction();
	
	// both versions of getNextSample reset the index when necessary.
	// The parameterless version assumes you don't need to know when the
	// end is reached, although you can use samplesRemain() to find this out.
	// to avoid making this function every time you call getNextSample(),
	// the other version does the comparison internally.
	
	//void getNextSample(float& outSample);
	
	// returns true if the last sample is reached
	bool getNextSample(float& outSample);
	
	bool samplesRemain() const;
	
	void setWindowType(unsigned int type);
	
	unsigned int getWindowType() const { return _windowType; }
	
	// set number of samples required, resets index
	void setWindowLength(unsigned int length);
	
	// functions to optimize (but not safe!)
	float* getWindowTable();
	double getCurrentIndex() const { return _index; }
	void setIndex(double index);
	int getWindowSize() const { return _windowSize; }
	int getWindowLength() const { return _length; }
	
	enum WindowTypes
	{
		kRectangle = 0,
		kTrapezoid,
		kTriangle,
		kGaussian,
		kHamming,
		kHann,
		kBlackman,
		kRampDown,
		kRampUp,
		kNumWindowTypes
	};
	
private:

	double _inc;
	double _index;
	unsigned int _length;
	unsigned int _windowType;
	unsigned int _windowSize;
	
	// only the first instance should call this.
	void _generateWindowTables();
};

#endif