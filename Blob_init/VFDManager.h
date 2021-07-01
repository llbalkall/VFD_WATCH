#ifndef VFDMANAGER_H_INCLUDED
#define VFDMANAGER_H_INCLUDED




#define OUT0  0x0002
#define OUT1  0x0004
#define OUT2  0x0008
#define OUT3  0x0010
#define OUT4  0x0020
#define OUT5  0x0040
#define OUT6  0x0080
#define OUT7  0x0100
#define OUT8  0x0200
#define OUT9  0x0400
#define OUT10 0x0800
#define OUT11 0x0001
                                    //   -- A -- 
#define segment_A     OUT11         //  |       |
#define segment_B     OUT8          //  F       B
#define segment_C     OUT2          //  |       |
#define segment_D     OUT3          //   -- G --
#define segment_E     OUT1          //  |       |  
#define segment_F     OUT6          //  E       C
#define segment_G     OUT9          //  |       |
                                    //   -- D --    

class VFDManager{
    public:
        unsigned int char_to_segments(char inputChar);
};

#endif //VFDMANAGER_H_INCLUDED
