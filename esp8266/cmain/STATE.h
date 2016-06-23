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

struct temp_t {
	bool state;
	int hilimit;
	int lolimit;
};

struct timr_t{
	bool state;
};

struct state_t{
	temp_t temp1;
	temp_t temp2;
	timr_t timr1;
	timr_t timr2;
	timr_t timr3;
	bool AUTOMA;
	bool sndStatus;
	bool sndSched;
  bool HAY_CNG;
  bool NEEDS_RESET;  	
};



#endif