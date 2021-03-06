#include <iostream>
#include <string>
#include <vector>
#include <exception>

#include <getopt.h>

#include "libgcm/config.hpp"

#if CONFIG_ENABLE_LOGGING
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/mdc.h>
#endif

#include <mpi.h>

#include "launcher/launcher.hpp"
#include "launcher/util/FileFolderLookupService.hpp"
#include "libgcm/Engine.hpp"

using namespace std;
using namespace gcm;
using namespace launcher;


// This function print usage message
void print_help(char* binaryName)
{
    cout << "\nUsage: " << binaryName << " --task file --data-dir dir\n"
        << "\t--task - xml file with task description\n"
        << "\t--data-dir - directory with models specified in task\n";
};

int main(int argc, char **argv, char **envp)
{
    FileFolderLookupService fls;

    // Parse command line options
    int c;
    static struct option long_options[] =
    {
        {"task"      , required_argument, 0, 't'},
        {"data-dir"  , required_argument, 0, 'd'},
        {"help"      , no_argument      , 0, 'h'},
        {0           , 0                , 0, 0  }
    };
    int option_index = 0;

    string taskFile;
    string dataDir;

    while ((c = getopt_long (argc, argv, "t:d:h:", long_options, &option_index)) != -1)
    {
        switch (c)
        {
            case 't':
                taskFile = optarg;
                break;
            case 'd':
                dataDir = optarg;
                if(dataDir[dataDir.length()-1] == '/')
                    dataDir[dataDir.length()-1] = '\0';
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            case '?':
                print_help(argv[0]);
            default:
                return -1;
        }
    }

    USE_AND_INIT_LOGGER("gcm");

    try {
        fls.addPath(CONFIG_SHARE_GCM);
        if( dataDir.empty() )
            dataDir = CONFIG_SHARE_GCM;
        fls.addPath(dataDir);
        fls.addPath("./src/launcher/");


        MPI::Init();
        #if CONFIG_ENABLE_LOGGING
        char pe[5];
        sprintf(pe, "%d", MPI::COMM_WORLD.Get_rank());
        log4cxx::MDC::put("PE", pe);
        log4cxx::PropertyConfigurator::configure(fls.lookupFile("log4cxx.properties"));
        #endif
    
        if( taskFile.empty() )
            THROW_INVALID_ARG("No task file provided");
        LOG_INFO("Starting with taskFile '" << taskFile << "' and dataDir '" << dataDir << "'");

        Engine& engine = Engine::getInstance();
        FileFolderLookupService::getInstance().addPath(dataDir);
        launcher::Launcher launcher;
        //launcher.loadMaterialLibrary("materials");
        launcher.loadSceneFromFile(taskFile);
        engine.calculate();
        engine.cleanUp();
        MPI::Finalize();

    } catch (Exception &e) {
        LOG_FATAL("Exception was thrown: " << e.getMessage() << "\n @" << e.getFile() << ":" << e.getLine() << "\nCall stack: \n"<< e.getCallStack());
    }
}
