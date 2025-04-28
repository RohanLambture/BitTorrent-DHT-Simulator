#!/usr/bin/env python3
# filepath: config.py

import re
import os

def read_topo_file(filename="topo.txt"):
    """Read the topology file and extract parameters."""
    try:
        with open(filename, 'r') as f:
            content = f.read()
            
        # Extract number of clients
        num_clients_match = re.search(r'num_clients\s*=\s*(\d+)', content)
        num_clients = int(num_clients_match.group(1)) if num_clients_match else 10  # default 10
        
        # Extract task sizes if present
        task_sizes_match = re.search(r'task_sizes\s*=\s*"([^"]+)"', content)
        task_sizes = task_sizes_match.group(1) if task_sizes_match else "4,5"  # default "4,5"
        
        # Extract connection delay if present
        delay_match = re.search(r'delay\s*=\s*(\d+(?:\.\d+)?)ms', content)
        delay = delay_match.group(1) if delay_match else "100"  # default 100ms
        
        # You could extract more parameters as needed
        
        return {
            "num_clients": num_clients,
            "task_sizes": task_sizes,
            "delay": delay
        }
    except FileNotFoundError:
        print(f"Warning: {filename} not found, using default values")
        return {
            "num_clients": 10,
            "task_sizes": "4,5",
            "delay": "100"
        }

def generate_connections(num_clients):
    """Generate connection code for clients based on Chord finger tables."""
    connections = []
    
    # For each client, generate its finger table connections
    for i in range(num_clients):
        # Formula for chord finger table: node connects to (n + 2^j) mod N
        # where j ranges from 0 to log2(N)-1
        finger_indices = []
        for j in range(int.bit_length(num_clients - 1)):
            finger = (i + (2**j)) % num_clients
            finger_indices.append(finger)
            connections.append(f"        client[{i}].out[{finger}] --> DelayChannel --> client[{finger}].in[{i}];")
            connections.append(f"        client[{i}].gout[{finger}] --> DelayChannel --> client[{finger}].gin[{i}];")
    
    return "\n".join(connections)

def generate_network_ned(params):
    """Generate the Network.ned file content."""
    
    ned_content = f"""// B22CS081_B22CS030 This file was generated automatically based on the configuration in topo.txt

simple ServerNode
{{
    parameters:
        int id;
        int numClients;  // Used to size gate vectors
    gates:
        input in[numClients];
        output out[numClients];
}}

simple ClientNode
{{
    parameters:
        int id;
//        int numServers;
        int numClients;
        string taskSizes;
    gates:
        // For task messages with servers:
        input in[numClients];
        output out[numClients];
        // For gossip messages with other clients:
        input gin[numClients];
        output gout[numClients];
}}

network Assignment2
{{
    parameters:
        int numClients = default({params['num_clients']});
    
    types:
        channel DelayChannel extends ned.DelayChannel {{
            delay = {params['delay']}ms;
        }}
    submodules:
        client[numClients]: ClientNode {{
            parameters:
                id = index;
                numClients = parent.numClients;
                taskSizes = "{params['task_sizes']}";
        }}

    connections allowunconnected:
        // Connect each node to its fingers using explicit indices
{generate_connections(params['num_clients'])}
}}
"""
    return ned_content

def write_network_ned(content, filename="Network.ned"):
    """Write the Network.ned file."""
    with open(filename, 'w') as f:
        f.write(content)
    print(f"Successfully generated {filename}")

def main():
    # Read the topology file
    params = read_topo_file()
    
    # Generate the Network.ned content
    ned_content = generate_network_ned(params)
    
    # Write the Network.ned file
    write_network_ned(ned_content)

if __name__ == "__main__":
    main()