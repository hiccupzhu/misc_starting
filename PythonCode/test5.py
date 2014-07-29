import os

def traverdir(path,other_path):
    if(os.path.isdir(path)):
        print other_path;
        try:
            os.mkdir(other_path);
        except:
            pass;
        for item in os.listdir(path):
            traverdir(path + '\\' + item,other_path + '\\' + item);
    else:
        filename=os.path.basename(path);
        if(filename.find('.') >= 0):
            if('h' == filename[filename.find('.')+1:]) or \
                ('def' == filename[filename.find('.')+1:]) or \
                ('dll' == filename[filename.find('.')+1:]) or \
                ('exp' == filename[filename.find('.')+1:]) or \
                ('lib' == filename[filename.find('.')+1:]):                
                dirPath = other_path;                
                sourcefile = open(path,'rb');                    
                lines = sourcefile.readlines();
                sourcefile.close();                        
                destinfile = open(dirPath,'wb');
                destinfile.writelines(lines);
                destinfile.close();                       
                            
                print path,
                print filename;
            
        
if __name__ == '__main__':
    source = 'F:\\MeCode\\VCCODE2\\ffmpeg3';
    destination = 'f:\\temp';    
    traverdir(source,destination + '\\' + os.path.basename(source));
