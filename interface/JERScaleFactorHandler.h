#ifndef JERSCALEFACTORHANDLER_H
#define JERSCALEFACTORHANDLER_H

#include <vector>
#include <utility>
#include "TDirectory.h"
#include "TFile.h"
#include "TString.h"
#include "ScaleFactorHandlerBase.h"
#include "JESRHelpers.h"
#include "JetResolution.h"
#include "AK4JetObject.h"
//#include "AK8JetObject.h"


class JERScaleFactorHandler : public ScaleFactorHandlerBase{
protected:
  // These are only for the MC
  JetResolution resolution_pt;
  //JetResolution resolution_phi;
  JetResolutionScaleFactor resolution_sf;

public:
  JESRHelpers::JetType const type;

  JERScaleFactorHandler(JESRHelpers::JetType type_);
  ~JERScaleFactorHandler();

  bool setup();
  void reset();

  bool applyJER(AK4JetObject* jet, float const& rho) const;
  //bool applyJER(AK8JetObject* jet, float const& rho) const;

};


#endif
