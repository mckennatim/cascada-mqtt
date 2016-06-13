#ifndef Cmd_h
#define Cmd_h

#include "STATE.h"

class Cmd{
public:
	bool deserialize(char* kstr); 
	void act(STATE& st);
private:
  bool heat;
  bool automa;
  int hilimit;
  int lolimit;
  bool empty;		
};

#endif