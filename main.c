#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>

#include "namespace_di_generated.h"
#include "namespace_machinery_generated.h"
#include "namespace_pumps_generated.h"

#include "example.h"

int main(int argc, char **argv) {
  /* Create a server */
  UA_Server *server = UA_Server_new();

  /* Create nodes from nodeset */
  UA_StatusCode status = UA_STATUSCODE_GOOD;
  status = namespace_di_generated(server);
  if (status != UA_STATUSCODE_GOOD) {
    UA_LOG_ERROR(
        UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
        "Adding the DI namespace failed. Please check previous error output.");
    UA_Server_delete(server);
    return EXIT_FAILURE;
  }
  status |= namespace_machinery_generated(server);
  if (status != UA_STATUSCODE_GOOD) {
    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                 "Adding the Machinery namespace failed. Please check previous "
                 "error output.");
    UA_Server_delete(server);
    return EXIT_FAILURE;
  }
  status |= namespace_pumps_generated(server);
  if (status != UA_STATUSCODE_GOOD) {
    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                 "Adding the Pumps namespace failed. Please check previous "
                 "error output.");
    UA_Server_delete(server);
    return EXIT_FAILURE;
  }

  /* Add new namespace */
  UA_Int16 myNameSpace = UA_Server_addNamespace(server, "MyNamespace");
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
              "New Namespace added with index %d", myNameSpace);

  /* Add a new pump object */
  addPumpTypeConstructor(server);
  addPumpObjectInstance(server, myNameSpace, "MyNewPump");
  // addCurrentTimeVariable(server);
  addDifferentialPressureVariable(server, myNameSpace);
  // addFlowRateDataSourceVariable(server);
  // addFlowRateExternalDataSource(server);

  /* Run the server */
  status = UA_Server_runUntilInterrupt(server);

  /* Clean up */
  UA_Server_delete(server);
  return status == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}