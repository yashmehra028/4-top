// This is the file "SimpleJetCorrectionUncertainty.cc". 
// This is the implementation of the class SimpleJetCorrectionUncertainty.
// Author: Konstantinos Kousouris, Philipp Schieferdecker
// Email:  kkousour@fnal.gov, philipp.schieferdecker@cern.ch

#include "SimpleJetCorrectionUncertainty.h"
#include "JetCorrectorParameters.h"
#include "JetCorrectionUtilities.h"
#include <vector>
#include <string>


using namespace JetCorrectionUtilities;


/////////////////////////////////////////////////////////////////////////
SimpleJetCorrectionUncertainty::SimpleJetCorrectionUncertainty()
{
  mParameters = new JetCorrectorParameters();
}
/////////////////////////////////////////////////////////////////////////
SimpleJetCorrectionUncertainty::SimpleJetCorrectionUncertainty(const std::string& fDataFile)
{
  mParameters = new JetCorrectorParameters(fDataFile);
}
/////////////////////////////////////////////////////////////////////////
SimpleJetCorrectionUncertainty::SimpleJetCorrectionUncertainty(const JetCorrectorParameters& fParameters)
{
  mParameters = new JetCorrectorParameters(fParameters);
}
/////////////////////////////////////////////////////////////////////////
SimpleJetCorrectionUncertainty::~SimpleJetCorrectionUncertainty()
{
  delete mParameters;
}
/////////////////////////////////////////////////////////////////////////
float SimpleJetCorrectionUncertainty::uncertainty(std::vector<float> const& fX, float fY, bool fDirection) const
{
  float result = 1.;
  int bin = mParameters->binIndex(fX);
  if (bin<0) {
    //throw cms::Exception("SimpleJetCorrectionUncertainty")<<" bin variables out of range";
    //handleError("SimpleJetCorrectionUncertainty"," bin variables out of range");
    // give warning instead of crashing code..
    std::cout << "SimpleJetCorrectionUncertainty::uncertainty: WARNING: bin variables out of range" << std::endl;
    return 0.;
  }

  result = uncertaintyBin((unsigned) bin, fY, fDirection);
  return result;
}
/////////////////////////////////////////////////////////////////////////
float SimpleJetCorrectionUncertainty::uncertaintyBin(unsigned fBin, float fY, bool fDirection) const
{
  if (fBin >= mParameters->size())
    //throw cms::Exception("SimpleJetCorrectionUncertainty")<<" wrong bin: "<<fBin<<": only "<<mParameters->size()<<" are available";
    handleError("SimpleJetCorrectionUncertainty", " wrong bin");
  const std::vector<float>& p = mParameters->record(fBin).parameters();
  if ((p.size() % 3) != 0)
    //throw cms::Exception ("SimpleJetCorrectionUncertainty")<<"wrong # of parameters: multiple of 3 expected, "<<p.size()<< " got";    
    handleError("SimpleJetCorrectionUncertainty", " wrong # of parameters: multiple of 3 expected");
  unsigned int N = p.size()/3;
  std::vector<float> yGrid(N, 0.);
  std::vector<float> value(N, 0.);
  float result = -1.0;
  for (unsigned i=0; i<N; i++)
  {
    unsigned ind = 3*i;
    yGrid[i] = p[ind];
    if (fDirection)// true = UP
      value[i] = p[ind+1];
    else // false = DOWN
      value[i] = p[ind+2];
  }
  if (fY <= yGrid[0])
    result = value[0];
  else if (fY >= yGrid[N-1])
    result = value[N-1];
  else
  {
    int bin = findBin(yGrid, fY);
    float vx[2], vy[2];
    for (int i=0; i<2; i++)
    {
      vx[i] = yGrid[bin+i];
      vy[i] = value[bin+i];
    }
    result = linearInterpolation(fY, vx, vy);
  }
  return result;
}
/////////////////////////////////////////////////////////////////////////
float SimpleJetCorrectionUncertainty::linearInterpolation(float fZ, const float fX[2], const float fY[2]) const
{
  // Linear interpolation through the points (x[i],y[i]). First find the line that
  // is defined by the points and then calculate the y(z).
  float r = 0;
  if (fX[0] == fX[1])
  {
    if (fY[0] == fY[1])
      r = fY[0];
    else
      //throw cms::Exception("SimpleJetCorrectionUncertainty")<<" interpolation error";
      handleError("SimpleJetCorrectionUncertainty", " interpolation error");
  }
  else
  {
    float a = (fY[1]-fY[0])/(fX[1]-fX[0]);
    float b = (fY[0]*fX[1]-fY[1]*fX[0])/(fX[1]-fX[0]);
    r = a*fZ+b;
  }
  return r;
}
/////////////////////////////////////////////////////////////////////////
int SimpleJetCorrectionUncertainty::findBin(std::vector<float> const& v, float x) const
{
  int i;
  int n = v.size()-1;
  if (n<=0) return -1;
  if (x<v[0] || x>=v[n])
    return -1;
  for (i=0; i<n; i++)
  {
    if (x>=v[i] && x<v[i+1])
      return i;
  }
  return 0;
}
