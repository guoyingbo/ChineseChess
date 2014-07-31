#include "EngineUCCI.h"
#include <string>

EngineUCCI g_engine;

EngineUCCI::EngineUCCI(void)
{
    piple_in[0] = NULL;
    piple_in[1] = NULL;
    piple_out[0] = NULL;
    piple_out[1] = NULL;
    g_hInputFile = NULL;
}


EngineUCCI::~EngineUCCI(void)
{
    CloseHandle(piple_in[0]);
    CloseHandle(piple_in[1]);
    CloseHandle(piple_out[0]);
    CloseHandle(piple_out[1]);
}


bool EngineUCCI::CreateChildProcess(const char *file)
    // Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = piple_out[1];
    siStartInfo.hStdOutput = piple_out[1];
    siStartInfo.hStdInput = piple_in[0];
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    siStartInfo.wShowWindow = SW_HIDE;

    // Create the child process. 

    bSuccess = CreateProcess(NULL, 
        (LPSTR)file,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application. 
    if (  bSuccess ) 
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        return true;
    }
    return false;
}


bool EngineUCCI::open(const char *file )
{
    SECURITY_ATTRIBUTES saAttr; 

    // Set the bInheritHandle flag so pipe handles are inherited. 

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

    // Create a pipe for the child process's STDOUT. 

    if ( ! CreatePipe(&piple_out[0], &piple_out[1], &saAttr, 0) ) 
        return false; 

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if ( ! SetHandleInformation(piple_out[0], HANDLE_FLAG_INHERIT, 0) )
        return false; 

    // Create a pipe for the child process's STDIN. 

    if (! CreatePipe(&piple_in[0], &piple_in[1], &saAttr, 0)) 
       return false; 

    // Ensure the write handle to the pipe for STDIN is not inherited. 

    if ( ! SetHandleInformation(piple_in[1], HANDLE_FLAG_INHERIT, 0) )
       return false; 

    // Create the child process. 

    return CreateChildProcess(file);

}

bool EngineUCCI::write( const char *inStr )
{
    DWORD dwLen, dwWritten; 
    BOOL bSuccess = FALSE;

    dwLen = strlen(inStr);
    cocos2d::CCLog("Write: %s",inStr);
    bSuccess = WriteFile(piple_in[1], inStr, dwLen, &dwWritten, NULL);
    
    return bSuccess;
    


}

bool EngineUCCI::read( char *outStr )
{
    DWORD dwRead, dwWritten; 

    return (ReadFile( piple_out[0], outStr, 4096, &dwRead, NULL));
}

void EngineUCCI::flush(char *hope)
{
    DWORD dwRead, dwWritten; 
    char buf[4096] = "";
    std::string end;
    do 
    {
        memset(buf,0,4096);
        getLine(buf);
        char* ptr = buf +strlen(buf)-strlen(hope);
        end = ptr;
    } while (end != hope);
    
}

bool EngineUCCI::getLine( char *line )
{
    DWORD dwRead, dwWritten; 
    char *buf = line;
    char* hope = "\r\n";
    std::string end;
    char* ptr = buf;
    do 
    {
        ReadFile( piple_out[0], ptr, 1, &dwRead, NULL);
        ptr++;
        end = buf+(strlen(buf) - 2);

    } while (end!=hope);

    cocos2d::CCLog("Read: %s",line);
    return true;
}

void EngineUCCI::exit()
{
    write("quit\n");
}
