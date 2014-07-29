import os
import sys

def traverdir(path,depth=1):
    prefix =  depth*'\t';
    isWriteHead = False;
    if(os.path.isdir(path)):
        
        if(path!=dirPath):
            outfile.write(prefix + '<Filter Name="'+os.path.basename(path)+'">\n');
            isWriteHead=True;        
        for item in os.listdir(path):
            if(os.path.basename(item)!='CVS'):
                traverdir(path + '\\' + item,depth+1);
        if(isWriteHead):
            outfile.write(prefix + '</Filter>\n');
            isWriteHead=False;
            
    else:
        filename=os.path.basename(path);
        flag = True;
        if(filename.find('.')!=-1):
            mystr = filename.split('.');
            flag = mystr[1]=='cpp';
            flag |= mystr[1]=='h';
            flag |= mystr[1]=='c';
        #print flag;
        if(flag):
            childPath = '.' + path[len(dirPath):];
            print childPath;
            outfile.write(prefix+'<File RelativePath="'+childPath+'"></File>\n');
     
        
if __name__ == '__main__':
    '''dirPath=os.getcwd();'''
    dirPath = 'F:\\mesource\\linux-2.6.7';
    outfile=open(dirPath + '\\' + os.path.basename(dirPath) + '.vcproj','w');
    outfile.write('<?xml version="1.0" encoding="gb2312"?>\n');
    outfile.write('<VisualStudioProject Keyword="Win32Proj" Name="'+os.path.basename(dirPath) + '" ProjectGUID="{FE50D712-0A81-486B-9335-4A0AE3B7E793}" ProjectType="Visual C++" RootNamespace="Test1" TargetFrameworkVersion="196613" Version="9.00">\n');
    outfile.write(' <Platforms>\n');
    outfile.write('     <Platform Name="Win32"/>\n');
    outfile.write(' </Platforms>\n');
    outfile.write(' <ToolFiles></ToolFiles>\n');
    outfile.write(' <Configurations>\n');
    outfile.write('     <Configuration CharacterSet="1" ConfigurationType="1" IntermediateDirectory="$(ConfigurationName)" Name="Debug|Win32" OutputDirectory="$(SolutionDir)$(ConfigurationName)">\n');
    outfile.write('         <Tool Name="VCPreBuildEventTool"/>\n');
    outfile.write('         <Tool Name="VCCustomBuildTool"/>\n');
    outfile.write('         <Tool Name="VCXMLDataGeneratorTool"/>\n');
    outfile.write('         <Tool Name="VCWebServiceProxyGeneratorTool"/>\n');
    outfile.write('         <Tool Name="VCMIDLTool"/>\n');
    outfile.write('         <Tool BasicRuntimeChecks="3" DebugInformationFormat="4" MinimalRebuild="true" Name="VCCLCompilerTool" Optimization="0" PreprocessorDefinitions="WIN32;_DEBUG;_CONSOLE" RuntimeLibrary="3" UsePrecompiledHeader="0" WarningLevel="3"/>\n');
    outfile.write('         <Tool Name="VCManagedResourceCompilerTool"/>\n');
    outfile.write('         <Tool Name="VCResourceCompilerTool"/>\n');
    outfile.write('         <Tool Name="VCPreLinkEventTool"/>\n');
    outfile.write('         <Tool GenerateDebugInformation="true" LinkIncremental="2" Name="VCLinkerTool" SubSystem="1" TargetMachine="1"/>\n');
    outfile.write('         <Tool Name="VCALinkTool"/>\n');
    outfile.write('         <Tool Name="VCManifestTool"/>\n');
    outfile.write('         <Tool Name="VCXDCMakeTool"/>\n');
    outfile.write('         <Tool Name="VCBscMakeTool"/>\n');
    outfile.write('         <Tool Name="VCFxCopTool"/>\n');
    outfile.write('         <Tool Name="VCAppVerifierTool"/>\n');
    outfile.write('         <Tool Name="VCPostBuildEventTool"/>\n');
    outfile.write('     </Configuration>\n');
                
    outfile.write(' </Configurations>\n');
    outfile.write(' <References></References>\n');
    outfile.write(' <Files>\n');
    traverdir(dirPath);
    outfile.write(' </Files>\n');
    outfile.write(' <Globals></Globals>\n');
    outfile.write('</VisualStudioProject>\n');
    outfile.close();
 
