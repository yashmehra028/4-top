#ifndef JETOBJECT_H
#define JETOBJECT_H

#define JET_EXTRA_VARIABLES \ 
JET_VARIABLE(float, Jet_btagDeepFlavB)\ 
JET_VARIABLE(float, Jet_jetId)

class JetVariables{
public:
  #define JET_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
    JET_VARIABLES;
  #undef JET_VARIABLE

  JetVariables();
  JetVariables(JetVariables const& other);
  JetVariables& operator=(const JetVariables& other);

  void swap(JetVariables& other);
};

class JetObject : public ParticleObject{
public:
  JetVariables extras;

  JetObject();
  JetObject(int id_);
  JetObject(int id_, LorentzVector_t const& mom_);
  JetObject(const JetObject& other);
  JetObject& operator=(const JetObject& other);
  ~JetObject();

  void swap(JetObject& other);
};

#endif
