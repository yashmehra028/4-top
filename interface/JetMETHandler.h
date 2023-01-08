#ifndef JETMETHANDLER_H
#define JETMETHANDLER_H

#include <vector>
#include "IvyBase.h"
#include "GenInfoHandler.h"
#include "SimEventHandler.h"
#include "MuonObject.h"
#include "ElectronObject.h"
#include "PhotonObject.h"
#include "AK4JetObject.h"
#include "METObject.h"
#include "ParticleDisambiguator.h"
#include "JECScaleFactorHandler.h"


class JetMETHandler : public IvyBase{
public:
  static const std::string colName_ak4jets;
  static const std::string colName_ak4jets_lowpt;
  static const std::string colName_pfmet;

protected:
  friend class ParticleDisambiguator;

  JECScaleFactorHandler* jecHandler_ak4jets;
  bool doComputeJECRCorrections;

  std::vector<AK4JetObject*> ak4jets;
  std::vector<AK4JetObject*> ak4jets_masked;

  METObject* pfmet;
  float pfmet_XYcorr_xCoeffA; float pfmet_XYcorr_xCoeffB;
  float pfmet_XYcorr_yCoeffA; float pfmet_XYcorr_yCoeffB;

  void clear();

  bool constructAK4Jets(SystematicsHelpers::SystematicVariationTypes const& syst, std::vector<GenJetObject*> const* genak4jets);
  bool constructAK4Jets_LowPt(SystematicsHelpers::SystematicVariationTypes const& syst, std::vector<GenJetObject*> const* genak4jets);
  bool constructMET(SystematicsHelpers::SystematicVariationTypes const& syst);

  bool assignMETXYShifts();

  bool computeJECRCorrections(AK4JetObject& obj, float const& rho, bool const& isData);

public:
  // Constructors
  JetMETHandler();

  // Destructors
  ~JetMETHandler(){ clear(); delete jecHandler_ak4jets; }

  bool constructJetMET(GenInfoHandler const* genInfoHandler, SimEventHandler const* simEventHandler, SystematicsHelpers::SystematicVariationTypes const& syst);

  std::vector<AK4JetObject*> const& getAK4Jets() const{ return ak4jets; }
  std::vector<AK4JetObject*> const& getMaskedAK4Jets() const{ return ak4jets_masked; }
  METObject* const& getPFMET() const{ return pfmet; }

  bool wrapTree(BaseTree* tree);

  void bookBranches(BaseTree* tree);

  void setComputeJECRCorrections(bool flag){ doComputeJECRCorrections = flag; }
};


#endif
