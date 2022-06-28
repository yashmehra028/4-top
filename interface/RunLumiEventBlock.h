#ifndef RUNEVENTLUMIBLOCK_H
#define RUNEVENTLUMIBLOCK_H


#define RUNLUMIEVENT_VARIABLES \
RUNLUMIEVENT_VARIABLE(unsigned int, RunNumber, run) \
RUNLUMIEVENT_VARIABLE(unsigned int, LuminosityBlock, luminosityBlock) \
RUNLUMIEVENT_VARIABLE(unsigned long long, EventNumber, event)


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
