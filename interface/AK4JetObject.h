#ifndef AK4JETOBJECT_H
#define AK4JETOBJECT_H

#include "ParticleObject.h"


#define AK4JET_GENINFO_VARIABLES \
AK4JET_VARIABLE(int, hadronFlavour, 0) \
AK4JET_VARIABLE(int, partonFlavour, 0)

#define AK4JET_COMMON_VARIABLES \
AK4JET_VARIABLE(int, jetId, 0) \
AK4JET_VARIABLE(int, puId, 0) \
AK4JET_VARIABLE(int, electronIdx1, -1) \
AK4JET_VARIABLE(int, electronIdx2, -1) \
AK4JET_VARIABLE(int, muonIdx1, -1) \
AK4JET_VARIABLE(int, muonIdx2, -1) \
AK4JET_VARIABLE(float, area, 0) \
AK4JET_VARIABLE(float, muonSubtrFactor, 0) \
AK4JET_VARIABLE(float, muEF, 0) \
AK4JET_VARIABLE(float, rawFactor, 0) \
AK4JET_VARIABLE(float, btagDeepFlavB, 0)

#define AK4JET_EXTRA_INPUT_VARIABLES \
AK4JET_GENINFO_VARIABLES \
AK4JET_COMMON_VARIABLES

#define AK4JET_EXTRA_VARIABLES \
AK4JET_EXTRA_INPUT_VARIABLES \
AK4JET_VARIABLE(float, JECL1Nominal, 1)


#define AK4JET_LOWPT_EXTRA_INPUT_VARIABLES \
AK4JET_LOWPT_VARIABLE(float, area, 0) \
AK4JET_LOWPT_VARIABLE(float, muonSubtrFactor, 0)

#define AK4JET_LOWPT_EXTRA_VARIABLES \
AK4JET_LOWPT_EXTRA_INPUT_VARIABLES \
AK4JET_LOWPT_VARIABLE(float, JECL1Nominal, 1)


class AK4JetVariables{
public:
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE

  AK4JetVariables();
  AK4JetVariables(AK4JetVariables const& other);
  AK4JetVariables& operator=(const AK4JetVariables& other);

  void swap(AK4JetVariables& other);
};

class AK4JetLowPtVariables{
public:
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  AK4JET_LOWPT_EXTRA_VARIABLES;
#undef AK4JET_LOWPT_VARIABLE

  AK4JetLowPtVariables();
  AK4JetLowPtVariables(AK4JetLowPtVariables const& other);
  AK4JetLowPtVariables& operator=(const AK4JetLowPtVariables& other);

  void swap(AK4JetLowPtVariables& other);
};


class AK4JetObject : public ParticleObject{
public:
  AK4JetVariables extras;
  AK4JetLowPtVariables extras_lowpt;

  AK4JetObject();
  AK4JetObject(LorentzVector_t const& mom_);
  AK4JetObject(const AK4JetObject& other);
  AK4JetObject& operator=(const AK4JetObject& other);
  ~AK4JetObject();

  void swap(AK4JetObject& other);
};

#endif
