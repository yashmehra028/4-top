#ifndef AK4JETOBJECT_H
#define AK4JETOBJECT_H

#include "ParticleObject.h"


#define AK4JET_GENINFO_VARIABLES \
AK4JET_VARIABLE(int, hadronFlavour)

#define AK4JET_COMMON_VARIABLES \
AK4JET_VARIABLE(int, jetId) \
AK4JET_VARIABLE(float, rawFactor) \
AK4JET_VARIABLE(float, btagDeepFlavB)

#define AK4JET_EXTRA_VARIABLES \
AK4JET_GENINFO_VARIABLES \
AK4JET_COMMON_VARIABLES


class AK4JetVariables{
public:
#define AK4JET_VARIABLE(TYPE, NAME) TYPE NAME;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE

  AK4JetVariables();
  AK4JetVariables(AK4JetVariables const& other);
  AK4JetVariables& operator=(const AK4JetVariables& other);

  void swap(AK4JetVariables& other);
};

class AK4JetObject : public ParticleObject{
public:
  AK4JetVariables extras;

  AK4JetObject();
  AK4JetObject(LorentzVector_t const& mom_);
  AK4JetObject(const AK4JetObject& other);
  AK4JetObject& operator=(const AK4JetObject& other);
  ~AK4JetObject();

  void swap(AK4JetObject& other);
};

#endif
