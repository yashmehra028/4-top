#ifndef GENINFOOBJECT_H
#define GENINFOOBJECT_H

#include <unordered_map>
#include <string>
#include <vector>
#include "StdExtensions.h"
#include "SystematicVariations.h"


#define GENINFO_EXTRA_VARIABLES \
GENINFO_VARIABLE(float, genHEPMCweight, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR1_muF1, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR1_muF2, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR1_muF0p5, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR2_muF1, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR2_muF2, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR2_muF0p5, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR0p5_muF1, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR0p5_muF2, 1) \
GENINFO_VARIABLE(float, LHEweight_QCDscale_muR0p5_muF0p5, 1) \
GENINFO_VARIABLE(float, LHEweight_PDFVariation_Up, 1) \
GENINFO_VARIABLE(float, LHEweight_PDFVariation_Dn, 1) \
GENINFO_VARIABLE(float, LHEweight_AsMZ_Up, 1) \
GENINFO_VARIABLE(float, LHEweight_AsMZ_Dn, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muRoneoversqrt2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muRoneoversqrt2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muRsqrt2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muRsqrt2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muR0p5, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muR0p5, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muR2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muR2, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muR0p25, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muR0p25, 1) \
GENINFO_VARIABLE(float, PythiaWeight_isr_muR4, 1) \
GENINFO_VARIABLE(float, PythiaWeight_fsr_muR4, 1)


class GenInfoVariables{
public:
#define GENINFO_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  GENINFO_EXTRA_VARIABLES;
#undef GENINFO_VARIABLE

  GenInfoVariables();
  GenInfoVariables(GenInfoVariables const& other);
  GenInfoVariables& operator=(const GenInfoVariables& other);

  void swap(GenInfoVariables& other);

};

class GenInfoObject{
public:
  GenInfoVariables extras;

public:
  GenInfoObject();
  GenInfoObject(const GenInfoObject& other);
  GenInfoObject& operator=(const GenInfoObject& other);
  ~GenInfoObject();

  void swap(GenInfoObject& other);

  void acquireWeightsFromArray(float* const& wgts);

  float getGenWeight(SystematicsHelpers::SystematicVariationTypes const& syst) const;
};

#endif
