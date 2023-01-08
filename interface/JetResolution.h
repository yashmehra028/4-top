#ifndef JETRESOLUTION_H
#define JETRESOLUTION_H

#include "JetResolutionObject.h"


class JetResolution{
private:
  std::shared_ptr<JetResolutionObject> m_object;

public:
  JetResolution(const std::string& filename);
  JetResolution(const JetResolutionObject& object);
  JetResolution(){}

  float getResolution(const JetParameters& parameters) const;

  JetResolutionObject const* getResolutionObject() const{ return m_object.get(); }
};

class JetResolutionScaleFactor{
private:
  std::shared_ptr<JetResolutionObject> m_object;

public:

  JetResolutionScaleFactor(const std::string& filename);
  JetResolutionScaleFactor(const JetResolutionObject& object);
  JetResolutionScaleFactor(){}

  float getScaleFactor(
    const JetParameters& parameters,
    JESRHelpers::JERVariation variation = JESRHelpers::JERVariation::kJERNominal,
    std::string uncertaintySource = ""
  ) const;

  JetResolutionObject const* getResolutionObject() const{ return m_object.get(); }
};


#endif
