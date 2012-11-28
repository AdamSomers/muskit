#ifndef h_Interpolaters
#define h_Interpolaters

// Interpolator
// ----------------
/// \brief Class with inline interpolation routines of various types
///

class Interpolator
{
public:
   Interpolator(int type = kInterpolationTypeLagrange3)
   : fType(type)
   {
      fType = type;
   }
   
   void SetType(int type)
   {
      fType = type;
   }
   
   enum InterpolationType
   {
      kInterpolationTypeNone = 0,
      kInterpolationTypeLinear,
      kInterpolationTypeLagrange2,
      kInterpolationTypeLagrange3,
      
      kNumInterpolationTypes
   };
   
   float Interpolate(float* inputBuf, double index, int bufferSize)
   {
      float input = inputBuf[(int)index];
      float output = input;
      if (fType == kInterpolationTypeLinear)
      {
         double delta = index - (int)index;
         float next = inputBuf[(int)(index+1) % bufferSize];
         output = inputBuf[(int)index] + delta * (next - input);
      }
      else if (fType == kInterpolationTypeLagrange2)
      {
         float next1 = inputBuf[(int)(index+1) % bufferSize];
         float next2 = inputBuf[(int)(index+2) % bufferSize];
         double delta = index - (int)index;
         double h0 = ((delta-1)*(delta-2))/2;
         double h1 = -delta*(delta-2);
         double h2 = (delta*(delta-1))/2;
         output = h0*input+h1*next1+h2*next2;
      }
      else if (fType == kInterpolationTypeLagrange3)
      {
         float next1 = inputBuf[(int)(index+1) % bufferSize];
         float next2 = inputBuf[(int)(index+2) % bufferSize];
         float next3 = inputBuf[(int)(index+3) % bufferSize];
         double delta = index - (int)index;
         double h0 = -((delta-1)*(delta-2)*(delta-3))/6;
         double h1 = (delta*(delta-2)*(delta-3))/2;
         double h2 = -(delta*(delta-1)*(delta-3))/2;
         double h3 = (delta*(delta-1)*(delta-2))/6;
         output = h0*input+h1*next1+h2*next2+h3*next3;
      }
      
      return output;
   }
   
private:
   int fType;
};

#endif