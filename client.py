#!/usr/bin/env python3
import time
import asyncio
from asyncua import Client

url = "opc.tcp://localhost:4840"
namespace = "MyNamespace"


async def main():

    print(f"Connecting to {url} ...", end=" ")
    async with Client(url=url) as client:
        print("connected!")

        NS_IDX_DI = await client.get_namespace_index("http://opcfoundation.org/UA/DI/")
        NS_IDX_MACHINERY = await client.get_namespace_index(
            "http://opcfoundation.org/UA/Machinery/"
        )
        NS_IDX_PUMPS = await client.get_namespace_index(
            "http://opcfoundation.org/UA/Pumps/"
        )
        NS_IDX_MYNS = await client.get_namespace_index(namespace)

        node = await client.nodes.root.get_child(
            f"0:Objects/{NS_IDX_MACHINERY}:Machines/{NS_IDX_MYNS}:MyNewPump"
        )
        name = await node.read_browse_name()
        print(f"Found node {node} with browse name {name.NamespaceIndex}:{name.Name}")

        rpaths = [
            f"{NS_IDX_DI}:Identification/{NS_IDX_DI}:Manufacturer",
            f"{NS_IDX_DI}:Identification/{NS_IDX_DI}:ProductInstanceUri",
            f"{NS_IDX_DI}:Identification/{NS_IDX_DI}:SerialNumber",
        ]
        for rp in rpaths:
            node = await client.nodes.root.get_child(
                f"0:Objects/{NS_IDX_MACHINERY}:Machines/{NS_IDX_MYNS}:MyNewPump/{rp}"
            )
            name = await node.read_browse_name()
            value = await node.read_value()
            print(f"{name.NamespaceIndex}:{name.Name} ({node}): {value}")

        node = await client.nodes.root.get_child(
            f"0:Objects/{NS_IDX_MACHINERY}:Machines/{NS_IDX_MYNS}:MyNewPump/{NS_IDX_DI}:Operational/{NS_IDX_PUMPS}:Measurements/{NS_IDX_PUMPS}:DifferentialPressure"
        )
        name = await node.read_browse_name()
        print(f"Found node ({node}) with browse name {name.NamespaceIndex}:{name.Name}")

        print("Listening...")
        while True:
            value = await node.read_value()
            print(f"{value}")
            time.sleep(2)


if __name__ == "__main__":
    asyncio.run(main())
