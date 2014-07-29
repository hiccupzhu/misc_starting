import os

def traverdir(path):
    num=0;
    if(os.path.isdir(path)):
        for item in os.listdir(path):
            traverdir(path + '/' + item);
    else:
        filename=os.path.basename(path);
        print filename;
        num = num +1;
        
        os.rename(path + '/' + filename,str(num) + '.log');
if __name__ == '__main__':
    traverdir('f:/text');
