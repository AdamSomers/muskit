
#include "WindowFunction.h"
#include <cmath>
#include "MathHelpers.h"

const float    float_E   = 2.71828182845904523536f; 

float** gWindowTables = 0;

WindowFunction::WindowFunction(unsigned int size)
{
	_windowSize = size;
	_index = 0;
	_length = 0;
	_inc = 0;
	_windowType = kRectangle;	
	setWindowLength(4096);  
	
	if (!gWindowTables) {
		_generateWindowTables();
		//gWindowTables = gWindowTables;
	}
}

WindowFunction::~WindowFunction()
{
}

bool WindowFunction::samplesRemain() const
{
	return (_index < _length);
}

// this could be sped up by requiring the caller to 
// maintain the flag, and we can set it via ref param
bool WindowFunction::getNextSample(float& outSample)
{
	outSample = gWindowTables[_windowType][(unsigned int)_index];
	_index += _inc;
	
	bool result = false;
	if (_index >= _windowSize) {
		result = true;
		_index = 0;
	}
		
	return result; 
}

//void WindowFunction::getNextSample(float& outSample)
//{	
//	outSample = gWindowTables[_windowType][(unsigned int)_index];
//	_index += _inc;
//	
//	if (_index >= _length)
//		_index = 0;
//}

void WindowFunction::setWindowLength(unsigned int length)
{
	_index = 0;
	_length = length;
//	_inc = length / (double)_windowSize;
	_inc = _windowSize / (double)length;
}

void WindowFunction::setWindowType(unsigned int type)
{
	_windowType = type;
}

float* WindowFunction::getWindowTable()
{
	return gWindowTables[_windowType];
}

void WindowFunction::setIndex(double index)
{
	_index = index;
}

void WindowFunction::_generateWindowTables()
{
	gWindowTables = new float*[kNumWindowTypes];
	for (int i = 0; i < kNumWindowTypes; i++) {
		gWindowTables[i] = new float[_windowSize];
		switch (i)
		{
		case kTriangle:
			{
				for (unsigned int j = 0; j < _windowSize; j++) {
					if (j < _windowSize / 2)
						gWindowTables[i][j] = j / (float)(_windowSize / 2);
					else
						gWindowTables[i][j] =  1 - ((j - (_windowSize / 2)) /  (float)(_windowSize / 2));
				}
			}
			break;
		case kTrapezoid:
			{
				unsigned int rampLength = 512;
				if (_windowSize <= 1024)
					rampLength = _windowSize / 8;
				for (unsigned int j = 0; j < _windowSize; j++) {
					if (j < rampLength)
						gWindowTables[i][j] = j / (float)(rampLength);
					else if (j < _windowSize - rampLength)
						gWindowTables[i][j] = 1.f;
					else
						gWindowTables[i][j] = 1 - ((j - (_windowSize - rampLength)) / (float)(rampLength));
				}
			}
			break;			
		case kRectangle:
			{
				 for (unsigned int j = 0; j < _windowSize; j++) {
						gWindowTables[i][j] = 1.f;
				}	
			}
			break;
		case kHamming:
			{
				 for (unsigned int j = 0; j < _windowSize; j++) {
					gWindowTables[i][j] = 0.53836 - 0.46164 * cos((2 * PI * j)/(float)(_windowSize - 1));
				}	
			}
			break;
		case kGaussian:
			{
				
				for (unsigned int j = 0; j < _windowSize; j++) {
					float x = (j-(_windowSize-1)/2.f) / (0.4*((_windowSize-1)/2.f));
					gWindowTables[i][j] = pow(float_E, (float)(-0.5 * pow(x, 2.f)));
				}	
			}
			break;
		case kHann:
			{
				 for (unsigned int j = 0; j < _windowSize; j++) {
					gWindowTables[i][j] = 0.5 * (1 - cos(2 * PI * j / (float)(_windowSize - 1)));
				}	
			}
			break;
		case kBlackman:
			{
				float a0 = 0.42;
				float a1 = 0.5;
				float a2 = 0.08;
				for (unsigned int j = 0; j < _windowSize; j++) {
					gWindowTables[i][j] = a0 - a1 * cos((2 * PI * j) / (_windowSize - 1)) + a2 * cos((4 * PI * j) / (_windowSize - 1));
				}	
			}
			break;		
		case kRampUp:
			{
				unsigned int rampLength = 128;
				if (_windowSize <= 256)
					rampLength = _windowSize / 8;
				for (unsigned int j = 0; j < _windowSize; j++) {
					if (j < _windowSize - rampLength)
						gWindowTables[i][j] = j / (float)(_windowSize - rampLength);
					else
						gWindowTables[i][j] = 1 - ((j - _windowSize + rampLength) / (float)rampLength);
				}
			}
			break;
		case kRampDown:
			{
				unsigned int rampLength = 128;
				if (_windowSize <= 256)
					rampLength = _windowSize / 8;
				for (unsigned int j = 0; j < _windowSize; j++) {
					if (j < rampLength)
						gWindowTables[i][j] = j / (float)(rampLength);
					else
						gWindowTables[i][j] = 1 - (j - rampLength) / (float)(_windowSize - rampLength);
				}
			}
			break;
		default:
			{
				for (unsigned int j = 0; j < _windowSize; j++) {
					if (j < _windowSize / 2)
						gWindowTables[i][j] = j / (_windowSize / 2);
					else
						gWindowTables[i][j] =  1 - ((j - (_windowSize / 2)) /  (_windowSize / 2));
				}
			}	
			break;	
		}
	}
}