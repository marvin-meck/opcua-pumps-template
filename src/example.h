#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>

UA_StatusCode addPumpObjectInstance(UA_Server *server, UA_Int16 nsParentIndex,
                                    char *name);

UA_StatusCode pumpTypeConstructor(UA_Server *server, const UA_NodeId *sessionId,
                                  void *sessionContext, const UA_NodeId *typeId,
                                  void *typeContext, const UA_NodeId *nodeId,
                                  void **nodeContext);

UA_StatusCode addPumpTypeConstructor(UA_Server *server);

// UA_DataValue *externalFlowValue;

void addOperationalComponent(UA_Server *server, UA_NodeId parentNodeId,
                             UA_NodeId *outNewNodeId);
void addMeasurementsComponent(UA_Server *server, UA_NodeId parentNodeId,
                              UA_NodeId *outNewNodeId);

void virtualPressureSensor(double *dp);
UA_StatusCode
readDifferentialPressure(UA_Server *server, const UA_NodeId *sessionId,
                         void *sessionContext, const UA_NodeId *nodeId,
                         void *nodeContext, UA_Boolean sourceTimeStamp,
                         const UA_NumericRange *range, UA_DataValue *dataValue);

void addDifferentialPressureVariable(UA_Server *server, UA_Int16 nsParentIndex);
// void ifferentialPressureExternalDataSource(UA_Server *server);
