#ifndef Cmd_h
#define Cmd_h

#include "STATE.h"
#include "PORTS.h"

class Cmd{
public:
	bool deserialize(char* kstr); 
	void act(STATE& st, PORTS& po);
private:
	bool temp1r;
	int temp1hi;
	int temp1lo;
	bool temp2r;
	int temp2hi;
	int temp2lo;
	bool timr1r;
	bool timr2r;
	bool timr3r;
  bool automa;
  bool empty;		
  bool heat;
  int hilimit;
  int lolimit;
};

#endif