#include <fcntl.h>
#include "measurement_utils.h"
#include "payload_utils.h"

void RequestModulesMeasurement(void* dp)
{
    bool isFinalPayload = IsFinalPayload((uint8_t*)dp);

    printf("Route payload to ModuleAnalyzer\n");
    memcpy(modules_data, dp, 4096);
    modules_payload_loaded_emit_underlying();

    if(isFinalPayload)
    {
        modules_report_ready_wait();
    }
    else
    {
        modules_payload_get_wait();
    }
}

int run(void)
{
    // TODO: are there any actual files for us to open?
    // fs_ctrl_open comes from FileServer/src/server.c
    int myFD = fs_ctrl_open("/etc/hostname", O_RDONLY);
    // "open" here seems to come from FileServerInterface.camkes: procedure FSI
    // which in turn is implemented in libsel4muslcsys/blob/master/src/sys_io.c
    //int myFD = open("/etc/hostname", O_RDONLY);

    memset(msmt_data, '0', 4096);
    printf("VM Dataport reset!\n");
    //strcpy(msmt_data, "This is a crossvm dataport test string");
    
    // prepare for a full payload of measurements
    HashedModuleMeasurement** measurements = malloc(46 * sizeof(HashedModuleMeasurement*));
    int numMeasurements = 0;

    // wait until the module analyzer is ready
    modules_analyzer_ready_wait();
    
    // wait until the Linux kernel module is ready
    msmt_module_ready_wait();
    printf("Measurement Module Ready Signal Received\n");


    while(1)
    {
        // wait until I should request a measurement
        // ...
        printf("Request Module Measurements\n");
        while(1)
        {
            msmt_component_done_emit_underlying();
            msmt_module_ready_wait();
            if(IsFinalPayload((uint8_t*)msmt_data))
            {
                printf("Request Analysis\n");
                RequestModulesMeasurement(msmt_data);
                printf("Report: %s\n", (char*)modules_data);
                break;
            }
            else
            {
                RequestModulesMeasurement(msmt_data);
            }
        }
        break;
    }

    fprintf(stderr,"Measurement Manager Component Finished\n");
    return 0;
}
