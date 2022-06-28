#ifndef HLTTRIGGERPATHOBJECT_H
#define HLTTRIGGERPATHOBJECT_H

#include <string>
#include <vector>
#include "IvyFramework/IvyDataTools/interface/IvyDataTypes.h"
#include "TriggerObject.h"


#define HLTTRIGGERPATH_VARIABLES \
HLTTRIGGERPATH_VARIABLE(std::string, name, "") \
HLTTRIGGERPATH_VARIABLE(bool, passTrigger, false) \
HLTTRIGGERPATH_VARIABLE(int, L1prescale, 1) \
HLTTRIGGERPATH_VARIABLE(int, HLTprescale, 1)


class HLTTriggerPathObject{
protected:
  bool flag_valid;
  ivy_listIndex_short_t uniqueIdentifier;
  std::vector<TriggerObject const*> passedTriggerObjects;
  std::vector<TriggerObject const*> failedTriggerObjects;

public:
#define HLTTRIGGERPATH_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  HLTTRIGGERPATH_VARIABLES;
#undef HLTTRIGGERPATH_VARIABLE

  HLTTriggerPathObject();
  HLTTriggerPathObject(HLTTriggerPathObject const& other);
  HLTTriggerPathObject& operator=(const HLTTriggerPathObject& other);

  void swap(HLTTriggerPathObject& other);

  void setValid(bool const& flag){ flag_valid = flag; }
  void setUniqueIdentifier(ivy_listIndex_short_t const& uid_){ uniqueIdentifier = uid_; }
  ivy_listIndex_short_t const& getUniqueIdentifier() const{ return uniqueIdentifier; }
  ivy_listIndex_short_t& getUniqueIdentifier(){ return uniqueIdentifier; }

  void setTriggerObjects(std::vector<TriggerObject*> const& triggerObjects);

  bool const& isValid() const{ return flag_valid; }

  std::vector<TriggerObject const*>& getPassedTriggerObjects(){ return this->passedTriggerObjects; }
  std::vector<TriggerObject const*> const& getPassedTriggerObjects() const{ return this->passedTriggerObjects; }

  std::vector<TriggerObject const*>& getFailedTriggerObjects(){ return this->failedTriggerObjects; }
  std::vector<TriggerObject const*> const& getFailedTriggerObjects() const{ return this->failedTriggerObjects; }

  std::vector<TriggerObject const*> getAssociatedTriggerObjects() const;

};


#endif
