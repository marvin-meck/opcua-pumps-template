#include <open62541/plugin/log_stdout.h>

#include "example.h"
#include "pumps_nodeids.h"

#define NS_INDEX_DI 2
#define NS_INDEX_MACHINES 3
#define NS_INDEX_PUMPS 4

UA_StatusCode addPumpObjectInstance(UA_Server *server, UA_Int16 nsParentIndex,
                                    char *name) {

  UA_NodeId myPumpId;
  UA_NodeId requestedNewNodeId = UA_NODEID_STRING(nsParentIndex, name);

  /*  "To comply with the Finding all Machines in a Server use case of OPC UA
      for Machinery, all Pumps shall be added to the 3:Machines Object de-
      fined in (OPC 40001-1)." (in OPC 40223 Companion Spec for Pumps and
      Vacuum Pumps, ch. 7.1 p. 32)
  */
  UA_NodeId parentNodeId = UA_NODEID_NUMERIC(NS_INDEX_MACHINES, 1001LU);
  UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);

  UA_QualifiedName browseName = UA_QUALIFIEDNAME(nsParentIndex, name);
  UA_NodeId typeDefinition =
      UA_NODEID_NUMERIC(NS_INDEX_PUMPS, UA_PUMPSID_PUMPTYPE);
  UA_ObjectAttributes myPumpAttr = UA_ObjectAttributes_default;
  myPumpAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);

  UA_Server_addObjectNode(server, requestedNewNodeId, parentNodeId,
                          referenceTypeId, browseName, typeDefinition,
                          myPumpAttr, NULL, &myPumpId);

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode pumpTypeConstructor(UA_Server *server, const UA_NodeId *sessionId,
                                  void *sessionContext, const UA_NodeId *typeId,
                                  void *typeContext, const UA_NodeId *nodeId,
                                  void **nodeContext) {

  // cJSON *root = (cJSON *)*nodeContext;

  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "New pump created!");

  UA_RelativePathElement rpeIdent;
  UA_RelativePathElement_init(&rpeIdent);
  rpeIdent.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  rpeIdent.isInverse = false;
  rpeIdent.includeSubtypes = false;
  rpeIdent.targetName = UA_QUALIFIEDNAME(NS_INDEX_DI, "Identification");

  UA_BrowsePath bpIdent;
  UA_BrowsePath_init(&bpIdent);
  bpIdent.startingNode = *nodeId;
  bpIdent.relativePath.elementsSize = 1;
  bpIdent.relativePath.elements = &rpeIdent;

  UA_BrowsePathResult bprIdent =
      UA_Server_translateBrowsePathToNodeIds(server, &bpIdent);
  if (bprIdent.statusCode != UA_STATUSCODE_GOOD || bprIdent.targetsSize < 1)
    return bprIdent.statusCode;

  if (bprIdent.targets[0].targetId.nodeId.identifierType == 0)
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "found node 'Identification' (ns=%d;i=%d)",
                bprIdent.targets[0].targetId.nodeId.namespaceIndex,
                bprIdent.targets[0].targetId.nodeId.identifier.numeric);

  UA_QualifiedName myBrowseNames[3];

  myBrowseNames[0] = UA_QUALIFIEDNAME(NS_INDEX_DI, "Manufacturer");
  myBrowseNames[1] = UA_QUALIFIEDNAME(NS_INDEX_DI, "ProductInstanceUri");
  myBrowseNames[2] = UA_QUALIFIEDNAME(NS_INDEX_DI, "SerialNumber");

  UA_Variant value;
  UA_BrowsePathResult bpr;
  char *convert[3];
  for (int i = 0; i < 3; ++i) {
    // UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "next");
    bpr = UA_Server_browseSimplifiedBrowsePath(
        server, bprIdent.targets[0].targetId.nodeId, 1, &myBrowseNames[i]);

    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1)
      return bpr.statusCode;

    convert[i] =
        (char *)UA_malloc(sizeof(char) * myBrowseNames[i].name.length + 1);
    memcpy(convert[i], myBrowseNames[i].name.data,
           myBrowseNames[i].name.length);
    convert[i][myBrowseNames[i].name.length] = '\0';

    /* Set the value */
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "writing node '%s' (ns=%d,i=%d)", convert[i],
                bpr.targets[0].targetId.nodeId.namespaceIndex,
                bpr.targets[0].targetId.nodeId.identifier.numeric);

    //     cJSON *identification = cJSON_GetObjectItem(root, "Identification");

    if (i == 0) {
      // cJSON *manufacturer = cJSON_GetObjectItem(identification,
      // "Manufacturer"); cJSON *locale = cJSON_GetObjectItem(manufacturer,
      // "Locale"); cJSON *text = cJSON_GetObjectItem(manufacturer, "Text");
      UA_LocalizedText manufacturerName =
          UA_LOCALIZEDTEXT("en-US", "Beispiel GmbH");
      UA_Variant_setScalar(&value, &manufacturerName,
                           &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    } else if (i == 1) {
      // cJSON *uri = cJSON_GetObjectItem(identification, "ProductInstanceUri");
      UA_String productInstanceUri =
          UA_STRING("https://www.example.com/pumps#CNP1234");
      UA_Variant_setScalar(&value, &productInstanceUri,
                           &UA_TYPES[UA_TYPES_STRING]);
    } else if (i == 2) {
      // cJSON *serial_num = cJSON_GetObjectItem(identification,
      // "SerialNumber");
      UA_String serialNumber = UA_STRING("SN:123456789");
      UA_Variant_setScalar(&value, &serialNumber, &UA_TYPES[UA_TYPES_STRING]);
    }

    UA_Server_writeValue(server, bpr.targets[0].targetId.nodeId, value);
    // UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "done!");

    UA_BrowsePathResult_clear(&bpr);
    // UA_Variant_clear(&value);
  }

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode addPumpTypeConstructor(UA_Server *server) {
  /*
  see
  https://www.open62541.org/doc/1.3/nodestore.html#node-lifecycle-constructors-destructors-and-node-contexts
  "Every node carries a user-context and a constructor-context pointer.
  The user-context is used to attach custom data to a node. But the (user-
  defined) constructors and destructors may replace the user-context pointer
  if they wish to do so."
  */
  UA_NodeTypeLifecycle lifecycle;
  UA_NodeId pumpTypeId = UA_NODEID_NUMERIC(NS_INDEX_PUMPS, UA_PUMPSID_PUMPTYPE);

  lifecycle.constructor = pumpTypeConstructor;
  lifecycle.destructor = NULL;

  UA_StatusCode status =
      UA_Server_setNodeTypeLifecycle(server, pumpTypeId, lifecycle);

  return status;
}

void virtualPressureSensor(double *dp) {
  *dp = 10e5;
  float tmp = (float)rand() / (float)RAND_MAX - 0.5;
  *dp += tmp;
}

UA_StatusCode readDifferentialPressure(
    UA_Server *server, const UA_NodeId *sessionId, void *sessionContext,
    const UA_NodeId *nodeId, void *nodeContext, UA_Boolean sourceTimeStamp,
    const UA_NumericRange *range, UA_DataValue *dataValue) {

  double dp;
  virtualPressureSensor(&dp);

  UA_Variant_setScalarCopy(&dataValue->value, (UA_Double *)&dp,
                           &UA_TYPES[UA_TYPES_DOUBLE]);

  dataValue->hasValue = true;
  return UA_STATUSCODE_GOOD;
}

void addOperationalComponent(UA_Server *server, UA_NodeId parentNodeId,
                             UA_NodeId *outNewNodeId) {
  UA_ObjectAttributes attr = UA_ObjectAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT("en-US", "Operational");
  UA_NodeId requestedNodeId = UA_NODEID_STRING(NS_INDEX_DI, "Operational");
  UA_QualifiedName browseName = UA_QUALIFIEDNAME(NS_INDEX_DI, "Operational");
  UA_NodeId typeDefinition =
      UA_NODEID_NUMERIC(NS_INDEX_PUMPS, UA_PUMPSID_OPERATIONALGROUPTYPE);
  UA_Server_addObjectNode(server, requestedNodeId, parentNodeId,
                          UA_NS0ID(HASCOMPONENT), browseName, typeDefinition,
                          attr, NULL, outNewNodeId);
}

void addMeasurementsComponent(UA_Server *server, UA_NodeId parentNodeId,
                              UA_NodeId *outNewNodeId) {
  UA_ObjectAttributes attr = UA_ObjectAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT("en-US", "Measurements");
  UA_NodeId requestedNodeId = UA_NODEID_STRING(NS_INDEX_PUMPS, "Measurements");
  UA_QualifiedName browseName =
      UA_QUALIFIEDNAME(NS_INDEX_PUMPS, "Measurements");
  UA_NodeId typeDefinition =
      UA_NODEID_NUMERIC(NS_INDEX_PUMPS, UA_PUMPSID_MEASUREMENTSTYPE);

  UA_Server_addObjectNode(server, requestedNodeId, parentNodeId,
                          UA_NS0ID(HASCOMPONENT), browseName, typeDefinition,
                          attr, NULL, outNewNodeId);
}

void addDifferentialPressureVariable(UA_Server *server,
                                     UA_Int16 nsParentIndex) {

  UA_NodeId outNewNodeId;
  UA_NodeId parentNodeId = UA_NODEID_STRING(nsParentIndex, "MyNewPump");

  UA_VariableAttributes attr = UA_VariableAttributes_default;
  UA_Double dp = 10e5;
  UA_Variant_setScalar(&attr.value, &dp, &UA_TYPES[UA_TYPES_DOUBLE]);
  attr.displayName = UA_LOCALIZEDTEXT("en-US", "DifferentialPressure");
  attr.description = UA_LOCALIZEDTEXT("en-US", "DifferentialPressure in Pa");
  attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
  attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;

  UA_NodeId requestedNewNodeId =
      UA_NODEID_STRING(NS_INDEX_PUMPS, "DifferentialPressure");
  UA_QualifiedName browseName =
      UA_QUALIFIEDNAME(NS_INDEX_PUMPS, "DifferentialPressure");

  UA_DataSource dataSource;
  dataSource.read = readDifferentialPressure;
  dataSource.write = NULL;

  addOperationalComponent(server, parentNodeId, &outNewNodeId);
  addMeasurementsComponent(server, outNewNodeId, &outNewNodeId);

  //   UA_Server_addVariableNode(server, requestedNewNodeId, parentNodeId,
  //                             UA_NS0ID(HASCOMPONENT), browseName,
  //                             UA_NS0ID(BASEANALOGTYPE), attr, NULL, NULL);
  UA_Server_addDataSourceVariableNode(
      server, requestedNewNodeId, outNewNodeId, UA_NS0ID(HASCOMPONENT),
      browseName, UA_NS0ID(BASEANALOGTYPE), attr, dataSource, NULL, NULL);
  //   UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
  //               "DifferentialPrssure ID is ns=%d,s=%s\n",
  //               requestedNewNodeId.namespaceIndex,
  //               (char *)requestedNewNodeId.identifier.string.data);
}
