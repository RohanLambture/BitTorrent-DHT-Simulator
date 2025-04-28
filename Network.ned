// B22CS081_B22CS030 This file was generated automatically based on the configuration in topo.txt

simple ServerNode
{
    parameters:
        int id;
        int numClients;  // Used to size gate vectors
    gates:
        input in[numClients];
        output out[numClients];
}

simple ClientNode
{
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
}

network Assignment2
{
    parameters:
//        int numServers = default(4);
        int numClients = default(10);
    
    types:
        channel DelayChannel extends ned.DelayChannel {
            delay = 100ms;
        }
    
    submodules:
//        server[numServers]: ServerNode {
//            parameters:
//                id = index;
//                numClients = parent.numClients;
//        }
//        
        // Define client vector with conditionals for task sizes
        client[numClients]: ClientNode {
            parameters:
                id = index;
                numClients = parent.numClients;
                taskSizes = "4,5";
        }

    connections allowunconnected:
        // Gossip message connections between clients, avoiding duplicate connections:
        for i=0..numClients-1, for j=i+1..numClients-1 {
            //normal connections
            client[i].out[j] --> DelayChannel --> client[j].in[i];
            client[j].out[i] --> DelayChannel --> client[i].in[j];
            
            
            client[i].gout[j] --> DelayChannel --> client[j].gin[i];
            client[j].gout[i] --> DelayChannel --> client[i].gin[j];
        }
}
