#ifndef JERSCALEFACTORHANDLER_H
#define JERSCALEFACTORHANDLER_H


#include <vector>
#include "JESRHelpers.h"
#include "JetResolution.h"
#include "ScaleFactorHandlerBase.h"


class JERScaleFactorHandler : public ScaleFactorHandlerBase{
protected:
  JetResolution resolution_pt_data;
  JetResolution resolution_pt_mc;
  JetResolutionScaleFactor resolution_sf; // Only MC

public:
  JESRHelpers::JetType const type;

  JERScaleFactorHandler(JESRHelpers::JetType type_);
  ~JERScaleFactorHandler();

  bool setup();
  void reset();

  template<typename T> void smear(std::vector<T*>& jets, float const& rho, bool isMC);

};


#endif
