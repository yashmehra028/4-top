#ifndef JETMETHANDLER_H
#define JETMETHANDLER_H

#include <vector>
#include "IvyBase.h"
#include "SimEventHandler.h"
#include "AK4JetObject.h"
#include "METObject.h"


class JetMETHandler : public IvyBase{
public:
  static const std::string colName_ak4jets;
  static const std::string colName_ak4jets_lowpt;
  static const std::string colName_pfmet;

protected:
  std::vector<AK4JetObject*> ak4jets;
  std::vector<AK4JetObject*> ak4jets_masked;

  METObject* pfmet;
  float pfmet_XYcorr_xCoeffA; float pfmet_XYcorr_xCoeffB;
  float pfmet_XYcorr_yCoeffA; float pfmet_XYcorr_yCoeffB;

  void clear();

  bool constructAK4Jets();
  bool constructAK4Jets_LowPt();
  bool constructMET();

  bool assignMETXYShifts();

public:
  // Constructors
  JetMETHandler();

  // Destructors
  ~JetMETHandler(){ clear(); }

  bool constructJetMET(SimEventHandler const* simEventHandler);

  std::vector<AK4JetObject*> const& getAK4Jets() const{ return ak4jets; }
  std::vector<AK4JetObject*> const& getMaskedAK4Jets() const{ return ak4jets_masked; }
  METObject* const& getPFMET() const{ return pfmet; }

  bool wrapTree(BaseTree* tree);

  void bookBranches(BaseTree* tree);
};


#endif
