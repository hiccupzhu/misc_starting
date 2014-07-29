import os

tempPath='C:\Documents and Settings\Administrator\Cookies';

if __name__=='__main__':
    print tempPath;
    for item in os.listdir(tempPath):
        print item;
