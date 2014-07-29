import os
import sys

def del_file_folder(path):
    if(os.path.isdir(path)):
        for item in os.listdir(path):
            del_file_folder(os.path.join(path,item));
            try:
                os.rmdir(os.path.join(path,item));
            except:
                pass;
    else:
        print path;
        os.remove(path);

if __name__=='__main__':
    dirPath='';
    
    if(len(sys.argv)>2):
        dirPath=sys.argv[1];
        del_file_folder(dirPath);
    else:
        print 'the input path error,please try again.';

