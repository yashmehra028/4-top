#ifndef SYSTEMATICVARIATIONS_H
#define SYSTEMATICVARIATIONS_H

#include <string>
#include "TString.h"


namespace SystematicsHelpers{

  enum SystematicVariationTypes{
    sNominal = 0,

    tPDFScaleDn, tPDFScaleUp,
    tQCDScaleDn, tQCDScaleUp,
    tAsMZDn, tAsMZUp,
    tPDFReplicaDn, tPDFReplicaUp,
    tPythiaScaleDn, tPythiaScaleUp,
    tPythiaTuneDn, tPythiaTuneUp,

    eEleEffDn, eEleEffUp,
    eMuEffDn, eMuEffUp,
    ePhoEffDn, ePhoEffUp,

    eJECDn, eJECUp,
    eJERDn, eJERUp,
    ePUDn, ePUUp,
    ePUJetIdEffDn, ePUJetIdEffUp,
    eBTagSFDn, eBTagSFUp,

    eL1PrefiringDn, eL1PrefiringUp,

    eTriggerEffDn, eTriggerEffUp,

    nSystematicVariations,

    sUncorrected // For checks
  };

  bool isDownSystematic(SystematicsHelpers::SystematicVariationTypes const& type);
  bool isUpSystematic(SystematicsHelpers::SystematicVariationTypes const& type);
  SystematicsHelpers::SystematicVariationTypes getSystComplement(SystematicsHelpers::SystematicVariationTypes const& type);
  std::string getSystCoreName(SystematicsHelpers::SystematicVariationTypes const& type);
  std::string getSystName(SystematicsHelpers::SystematicVariationTypes const& type);

  double getLumiUncertainty_Uncorrelated();
  double getLumiUncertainty_Correlated();
  double getLumiUncertainty_Correlated_2015_2016();
  double getLumiUncertainty_Correlated_2017_2018();
}

#endif
