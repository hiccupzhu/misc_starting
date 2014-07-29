#ifndef __H_MRECT__
#define __H_MRECT__

class MRect{
public:
    RECT rc;
    
public:
    int Width(){
        return rc.right - rc.left;
    }
    
    int Height(){
        return rc.bottom - rc.top;
    }
    
    RECT* operator &(){
        return &rc;
    }

};

#endif