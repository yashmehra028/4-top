#include <cassert>
#include <algorithm>
#include <utility>
#include "AK4JetObject.h"
#include "BtagHelpers.h"
#include "AK4JetSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


AK4JetVariables::AK4JetVariables(){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
AK4JetVariables::AK4JetVariables(AK4JetVariables const& other){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
void AK4JetVariables::swap(AK4JetVariables& other){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
AK4JetVariables& AK4JetVariables::operator=(const AK4JetVariables& other){
  AK4JetVariables tmp(other);
  this->swap(tmp);
  return *this;
}

AK4JetObject::AK4JetObject() :
  ParticleObject(),
  extras()
{}
AK4JetObject::AK4JetObject(LorentzVector_t const& momentum_) :
  ParticleObject(0, momentum_),
  extras()
{}
AK4JetObject::AK4JetObject(const AK4JetObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void AK4JetObject::swap(AK4JetObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
AK4JetObject& AK4JetObject::operator=(const AK4JetObject& other){
  AK4JetObject tmp(other);
  this->swap(tmp);
  return *this;
}
AK4JetObject::~AK4JetObject(){}

BTagEntry::JetFlavor AK4JetObject::getBTagJetFlavor() const{
  auto const& jetFlavor = extras.hadronFlavour;
  if (std::abs(jetFlavor)==5) return BTagEntry::FLAV_B;
  else if (std::abs(jetFlavor)==4) return BTagEntry::FLAV_C;
  else return BTagEntry::FLAV_UDSG;
}
float AK4JetObject::getBtagValue() const{
  switch (AK4JetSelectionHelpers::btagger_type){
  case BtagHelpers::kDeepFlav_Loose:
  case BtagHelpers::kDeepFlav_Medium:
  case BtagHelpers::kDeepFlav_Tight:
    return this->extras.btagDeepFlavB;
  default:
    IVYerr << "AK4JetObject::getBtagValue: b-tagger type " << AK4JetSelectionHelpers::btagger_type << " is not implemented. Aborting..." << endl;
    assert(0);
  }
  return -1;
}
