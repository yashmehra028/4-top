#include <algorithm>
#include <utility>
#include <cmath>
#include "HLTTriggerPathObject.h"
#include "HelperFunctions.h"


HLTTriggerPathObject::HLTTriggerPathObject() :
  flag_valid(false),
  uniqueIdentifier(0)
{
#define HLTTRIGGERPATH_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  HLTTRIGGERPATH_VARIABLES;
#undef HLTTRIGGERPATH_VARIABLE
}
HLTTriggerPathObject::HLTTriggerPathObject(HLTTriggerPathObject const& other) :
  flag_valid(other.flag_valid),
  uniqueIdentifier(other.uniqueIdentifier),
  triggerObjects(other.triggerObjects)
{
#define HLTTRIGGERPATH_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  HLTTRIGGERPATH_VARIABLES;
#undef HLTTRIGGERPATH_VARIABLE
}
void HLTTriggerPathObject::swap(HLTTriggerPathObject& other){
  std::swap(this->flag_valid, other.flag_valid);
  std::swap(this->uniqueIdentifier, other.uniqueIdentifier);
  std::swap(this->triggerObjects, other.triggerObjects);
#define HLTTRIGGERPATH_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  HLTTRIGGERPATH_VARIABLES;
#undef HLTTRIGGERPATH_VARIABLE
}
HLTTriggerPathObject& HLTTriggerPathObject::operator=(const HLTTriggerPathObject& other){
  HLTTriggerPathObject tmp(other);
  swap(tmp);
  return *this;
}

void HLTTriggerPathObject::setTriggerObjects(std::vector<TriggerObject*> const& triggerObjects_){
  for (TriggerObject const* triggerObject:triggerObjects_){
    if (!HelperFunctions::checkListVariable(triggerObjects, triggerObject)) this->triggerObjects.push_back(triggerObject);
  }
}
