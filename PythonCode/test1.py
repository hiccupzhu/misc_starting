from os.path import basename, isdir
from os import listdir
import os

  
dir = 'F:/text';
print("-----------------------------------------");
outFile=open("e:/temp/test.txt","w");
def traverse(path, depth=0):
    
    prefix =  depth* '| ' + '|_'
    if(isdir(path)):
        '''
        outStr=prefix+basename(path);
        print outStr;
        outFile.write(outStr+'\n');
        '''
        for item in listdir(path):
            traverse(path+'/'+item, depth+1);
    else:
        '''
        outStr=prefix+basename(path);
        print outStr;
        '''
        f=open(basename(path),"r");
        frd=f.read();
        f.close();
        f=open(basename(path),"w");
        f.write("hello\n"+frd);
        f.flush();
        f.close();
        outFile.write(outStr+'\n');
    
    
if __name__ == '__main__':
    print("main");
    outFile.write(dir+"/\n");
    traverse(dir)
    outFile.flush();
    outFile.close();
