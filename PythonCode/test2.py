from os.path import basename, isdir
from os import listdir
import os


def traverse(path):
    
    if(isdir(path)):        
        for item in listdir(path):
            traverse(path+'/'+item);
    else:
        
        f1=open(path,"r");
        frd=f1.read();
        f1.close();
        f2=open(path,"w");
        outString = "/////////////////\nhello\n////////////////\n"+frd;
        f2.write(outString);
        f2.flush();
        f2.close();
        
    
    
if __name__ == '__main__':
    print("main");
    dir = 'F:/text';
    traverse(dir);
