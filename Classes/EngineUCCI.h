#ifndef WIN32
#define HANDLE int
#endif

#include "cocos2d.h"
class EngineUCCI
{
public:
    EngineUCCI(void);
    ~EngineUCCI(void);

    bool open(const char *file);
    void exit();
    bool write(const char *inStr);
    bool read(char *outStr);
    bool getLine(char *line);
    void flush(char *hope);
    bool CreateChildProcess(const char *file) /* Create a child process that uses the previously created pipes for STDIN and STDOUT. */;

protected:
    HANDLE piple_in[2];
    HANDLE piple_out[2];
    HANDLE g_hInputFile;
};

extern EngineUCCI g_engine;
