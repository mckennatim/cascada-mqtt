#ifndef STATE_h
#define STATE_h

struct STATE {
	int io2d4;
	int ALED;
	int temp1;
	int temp2;
	bool heat;
	int hilimit;
	int lolimit;
  bool AUTOMA;
  bool HAY_CNG;
  bool NEEDS_RESET;  
};

#endif