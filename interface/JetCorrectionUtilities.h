#ifndef JETCORRECTIONUTILITIES_H
#define JETCORRECTIONUTILITIES_H

#include <string>


namespace JetCorrectionUtilities{
  void handleError(const std::string& fClass, const std::string& fMessage);
  float getFloat(const std::string& token);
  unsigned getUnsigned(const std::string& token);
  long int getSigned(const std::string& token);
  std::string getSection(const std::string& token);
  std::vector<std::string> getTokens(const std::string& fLine);
  std::string getDefinitions(const std::string& token);
  float quadraticInterpolation(float fZ, const float fX[3], const float fY[3]);
}

#endif
