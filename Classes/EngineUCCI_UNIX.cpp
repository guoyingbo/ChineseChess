#include "EngineUCCI.h"
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include "cocos2d.h"
#include "eleeye.h"

EngineUCCI g_engine;

EngineUCCI::EngineUCCI(void)
{
    piple_in[0] = -1;
    piple_out[1] = -1;
}
EngineUCCI::~EngineUCCI(void)
{
}

 bool EngineUCCI::CreateChildProcess(const char *file) /* Create a child process that uses the previously created pipes for STDIN and STDOUT. */
 {
     int pid;
     pipe(piple_in);
     pipe(piple_out);
     pid = fork();
     if (pid == 0)
     {
         cocos2d::CCLog("%d %d %d d%",piple_in[0],piple_in[1],piple_out[0],piple_out[1]);

         dup2(piple_out[1],1);
         dup2(piple_in[0],0);

//         if (-1 == execl(file,"no",NULL))
//         {
//             cocos2d::CCLog("failed");
//            perror("execl error");
//         }
     //  cocos2d::CCDirector::sharedDirector()->end();
         eleeye();
         cocos2d::CCLog("child exit");
         ::exit(0);
     }
     else if(pid > 0)
     {
    //     cocos2d::CCLog("child process id %d",pid);
     }

     return true;
 }

 bool EngineUCCI::open(const char *file )
 {
     return CreateChildProcess(file);
 }


 bool EngineUCCI::write( const char *inStr )
 {
     long dwLen;


     dwLen = ::write(piple_in[1], inStr, strlen(inStr));

     cocos2d::CCLog("write %s",inStr);
     return dwLen;
 }

 bool EngineUCCI::read( char *outStr )
 {

     long dwLen = ::read( piple_out[0], outStr, 4096);
     cocos2d::CCLog("read %d",dwLen);
     return dwLen;
 }

 void EngineUCCI::flush(char *hope)
 {
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
     char end = 0;

     char* ptr = line;

     do 
     {
        if ( ::read( piple_out[0], ptr, 1)>0)
        {
            ptr++;
            end = line[(strlen(line) - 1)];
        }
        else
        {
            cocos2d::CCLog("-1");
        }
     } while (end != '\n');

     cocos2d::CCLog("%s",line);
     return true;
 }

 void EngineUCCI::exit()
 {
     write("quit\n");
 }
