#ifndef SIMPLEJETCORRECTIONUNCERTAINTY_H
#define SIMPLEJETCORRECTIONUNCERTAINTY_H

#include <string>
#include <vector>


class JetCorrectorParameters;


class SimpleJetCorrectionUncertainty{
public:
  SimpleJetCorrectionUncertainty();
  SimpleJetCorrectionUncertainty(const std::string& fDataFile);
  SimpleJetCorrectionUncertainty(const JetCorrectorParameters& fParameters);
  ~SimpleJetCorrectionUncertainty();
  const JetCorrectorParameters& parameters() const { return *mParameters; }
  float uncertainty(std::vector<float> const& fX, float fY, bool fDirection) const;

private:
  SimpleJetCorrectionUncertainty(const SimpleJetCorrectionUncertainty&);
  SimpleJetCorrectionUncertainty& operator= (const SimpleJetCorrectionUncertainty&);
  int findBin(std::vector<float> const& v, float x) const;
  float uncertaintyBin(unsigned fBin, float fY, bool fDirection) const;
  float linearInterpolation(float fZ, const float fX[2], const float fY[2]) const;
  JetCorrectorParameters* mParameters;
};

#endif
