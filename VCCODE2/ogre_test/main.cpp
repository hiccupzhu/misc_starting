#include <stdio.h>
#include <Windows.h>
#include "mogre.h"

static int quit = 0;

DWORD WINAPI ThreadProc(  LPVOID lpParameter){
    mogre* pm = (mogre*)lpParameter;
    while(quit == 0){
        pm->render_one_frame();
        Sleep(40);
    }
    return 0;
}

int main(int argc,char* argv[]){
    mogre m;
    m.init();
    m.load_entity();

    //m.init_sample();

    //HANDLE th_handle = CreateThread(NULL, 0, ThreadProc, &m, 0, 0);
    m.render_start();

    char ch = 0;
    for(;;){
        ch = getchar();
        if(ch == 'q' || ch == 'Q'){
            quit = 1;
            break;
        }
    }
    return 0;
}