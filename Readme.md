# OPCUA-PUMPS-Template

Template for compiling a OPC UA Server using the [`open62541`](https://www.open62541.org/) stack. 
A minimal client is also provided for debugging purposes. 
It is written in Python using the [opcua-asyncio](https://github.com/FreeOpcUa/opcua-asyncio) implementation. 

This template creates a new namespace "MyNamespace" (index 5), creates a pump object (`5:MyNewPump`) and places it into `/0:Objects/3:Machines` as required by the specification. 
The *ObjectType* *PumpType* has one mandatory component `2:Identification`, see [OPC 40223](https://reference.opcfoundation.org/Pumps/v100/docs/7.1). 
Values for `Manufacturer`, `ProductInstanceUri`, and `SerialNumber` are added for identification purposes. 

Next, a virtual differential pressure sensor is implemented at `/0:Objects/3:Machines/5:MyNewPump/2:Operational/4:Measurements/4:DifferentialSensor`. 
The implementation follows the [`open62541` documentation for connecting variables with a physical process](https://www.open62541.org/doc/v1.4.11.1/tutorial_server_datasource.html). 

## Dependencies

### open62541

Make sure to build `open62541` with the full namespace zero (UA_NAMESPACE_ZERO=FULL)[https://www.open62541.org/doc/v1.4.11.1/building.html#detailed-sdk-features]. 

`open62541` ("no longer expose[s] components via the CMake target")[https://github.com/open62541/open62541/issues/6597#issuecomment-2243014168] and so `cmake` won't exit out of the build process even if the full namespace zero is not available. 

Code base was developed and tested with `open62541 v1.4.11.1`.

### OPCFoundation/UA-Nodeset 

See https://github.com/OPCFoundation/UA-Nodeset. 

The `CMakeLists.txt` file accepts a variable named `UA_NODESET_DIR`.
You can either place the files from https://github.com/OPCFoundation/UA-Nodeset into the top-level directory or set the `UA_NODESET_DIR` variable to point to the location where these files are stored. 

*Note*: You will only need the `DI`, `Machinery` and `Pumps` nodesets. Also, I might add `UA-Nodeset` as a Git submodule in the future to simplify this setup. 