#ifndef STATE_h
#define STATE_h

struct STATE {
	int temp1;
	bool temp1r;
	int temp1hi;
	int temp1lo;
	int temp2;
	bool temp2r;
	int temp2hi;
	int temp2lo;
	bool timr1r;
	bool timr2r;
	bool timr3r;
  bool AUTOMA;
  bool HAY_CNG;
  bool NEEDS_RESET;  
};

#endif