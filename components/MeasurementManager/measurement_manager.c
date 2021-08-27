
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
    memset(dest, '0', 4096);
    printf("VM Dataport reset!\n");
    //strcpy(dest, "This is a crossvm dataport test string");

    // wait until the module analyzer is ready
    modules_analyzer_ready_wait();

    // wait until the Linux kernel module is ready
    ready_wait();
    printf("Measurement Module Ready Signal Received\n");

    // prepare for a full payload of measurements
    HashedModuleMeasurement** measurements = malloc(46 * sizeof(HashedModuleMeasurement*));
    int numMeasurements = 0;

    while(1)
    {
        printf("Request Measurement\n");
        done_emit_underlying();
        ready_wait();

        printf("Request Analysis\n");
        if(IsFinalPayload((uint8_t*)dest))
        {
            RequestModulesMeasurement(dest);
            printf("Report: %s\n", (char*)modules_data);
            break;
        }
        else
        {
            RequestModulesMeasurement(dest);
            modules_payload_get_wait();
        }
        break;
    }

    fprintf(stderr,"Measurement Manager Component Finished\n");
    return 0;
}
