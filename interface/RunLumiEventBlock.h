#ifndef RUNEVENTLUMIBLOCK_H
#define RUNEVENTLUMIBLOCK_H


typedef unsigned int RunNumber_t;
typedef unsigned int LuminosityBlock_t;
typedef unsigned long long EventNumber_t;

#define RUNLUMIEVENT_VARIABLES \
RUNLUMIEVENT_VARIABLE(RunNumber_t, RunNumber, run) \
RUNLUMIEVENT_VARIABLE(LuminosityBlock_t, LuminosityBlock, luminosityBlock) \
RUNLUMIEVENT_VARIABLE(EventNumber_t, EventNumber, event)


class RunLumiEventBlock{
protected:
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) TYPE NAME;
  RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE

public:
  RunLumiEventBlock();
  RunLumiEventBlock(unsigned int RunNumber_, unsigned int LuminosityBlock_, unsigned long long EventNumber_);
  RunLumiEventBlock(RunLumiEventBlock const& other);

  bool operator==(RunLumiEventBlock const& other) const;
};


#endif
