# Authors : B22CS081 B22CS030
# Computer Network Assignment-2: Remote Program Execution Simulation using OMNeT++

## Overview

This project simulates remote program execution using the OMNeT++ Discrete Event Simulator. In the simulation, a client node divides a computational task (for example, determining the maximum element in an array) into several subtasks and distributes them among multiple server nodes for parallel processing. The overall result is determined by a majority vote based on the responses received from the servers. Additionally, client nodes exchange performance ratings for the servers via a gossip protocol, which is subsequently used to optimize future task assignments.

## Assignment Objectives

- *Task Partitioning:*  
  - Divide an array of x integer elements (with x ≥ 2) into n approximately equal segments, ensuring each segment contains at least two elements.  
    > Note: It is assumed that x is always at least 2.
  - Dispatch each subtask to n/2 + 1 servers for execution.

- *Server Processing:*  
  - Each server processes its assigned subtask by determining the maximum value within its designated sub-array.
  - Up to n/4 servers may act maliciously by returning incorrect results.

- *Result Consolidation:*  
  - Client nodes aggregate the results of all subtasks using majority voting (for example, by selecting the highest value from the subarray outputs).

- *Gossip-Based Server Rating:*  
  - Servers are rated by clients with a score of 1 for an honest response and 0 for a malicious one.
  - Upon task completion, each client broadcasts a gossip message in the format <timestamp>:<IP>:<Score#> to all directly connected nodes.
  - Each client records the first occurrence of every unique gossip message and uses these scores to compute an average rating for each server.
  - In subsequent rounds, clients preferentially assign subtasks to the top n/2 + 1 servers based on these averaged ratings.

## Network Setup

- *Server Nodes:*  
  - The simulation includes a minimum of n (> 3) server nodes.
  - Servers may operate either honestly or maliciously, with the number of malicious nodes limited to less than n/4.

- *Client Nodes:*  
  - Each client divides the overall task into n subtasks and dispatches each subtask to n/2 + 1 servers.
  - Clients collect the results, apply majority voting, and compute a consolidated final outcome.
  - Based on the results, an average score is assigned to each server and a corresponding gossip message is generated.
  - Full connectivity among clients is maintained to facilitate the exchange of server performance ratings via the gossip protocol.

- *Topology Configuration:*  
  - The network topology is defined in the topo.txt file.
  - This file can be modified during evaluation as needed to update the network configuration.

## Simulation Workflow

1. *Task Execution:*  
   - Clients partition the primary task (such as calculating the maximum value) into n subtasks.
   - Each subtask is assigned to a selected set of n/2 + 1 servers.
   - Servers compute the maximum value from their respective subarrays and return the result.

2. *Result Aggregation:*  
   - Clients determine the overall outcome by applying a majority vote (or by selecting the maximum among all responses).
   - The results are logged to both the console and output files.

3. *Server Rating and Gossip Protocol:*  
   - Servers receive a rating of 1 for honest responses and 0 for malicious responses.
   - Each client compiles these ratings and broadcasts a gossip message in the format <timestamp>:<IP>:<Score#>.
   - Unique gossip messages are forwarded exactly once, each accompanied by the sender's IP address and a local timestamp.
   - The aggregated gossip messages are used to calculate the average score for each server, thereby influencing task assignments in subsequent simulation rounds.

4. *Logging Mechanism:*  
   - The following information is logged to output.txt:
     - Each server writes the result of each computed subtask to both the console and an output file.
     - Each client writes the result of every subtask received from the servers, along with the consolidated result of each task, to the console and an output file.
     - Every unique gossip message, along with a local timestamp and the sender's IP address (node ID), is displayed on the console and logged to an output file.
     - Additionally, clients are capable of displaying the scores of each server on both the console and in a file.

## Program Output

- *Server Output:*  
  - Each server displays the result of its subtask on the console and logs the output to a file.

- *Client Output:*  
  - Each client displays the outcomes of all received subtasks along with the consolidated final result.
  - Gossip messages, including timestamps and sender IP addresses, are output to the console and recorded in a log file.
  - Consolidated server scores are also presented on the console and saved to an output file.

## Compilation and Execution Instructions

### Prerequisites

- *OMNeT++ Installation:*  
  Ensure that OMNeT++ is installed on your system. For installation instructions, please refer to the [OMNeT++ Install Guide](https://doc.omnetpp.org/omnetpp/InstallGuide.pdf).

- *C++ Compiler:*  
  A compatible C++ compiler is required to build the simulation project.

- *Python:*  
  Python is required to run the provided configuration script.

- *Environment Setup:*  
  Configure the necessary environment variables for OMNeT++.

### Steps to Compile and Run the Simulation

1. Open a terminal and navigate to the OMNeT++ installation root directory.
2. Launch OMNeT++ by executing:
   bash
   omnetpp
   
3. Within the OMNeT++ IDE, select the CNAssignment2 project.
4. Update the network topology in the topo.txt file as needed, and run the provided Python script (e.g., config.py) to generate the Network.ned file:
   bash
   python config.py
   
5. Finally, start the simulation by executing the omnetpp.ini file, which initiates the simulation process.

## File Structure

- `README.md`: This comprehensive documentation file.
- `Makefile`: Contains the build instructions for compiling the simulation project.
- `topo.txt`: Defines the network topology.
- Source code files (e.g., `*.cc` and `*.h`) that implement the simulation logic.
- OMNeT++ NED files (e.g., `*.ned`) that define the network topology or enable dynamic module creation.
- The `config.py` file, which dynamically generates the NED file based on the network topology specified in topo.txt.