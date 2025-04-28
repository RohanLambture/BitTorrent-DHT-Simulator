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
        int numClients = default(10);
    
    types:
        channel DelayChannel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        client[numClients]: ClientNode {
            parameters:
                id = index;
                numClients = parent.numClients;
                taskSizes = "4,5";
        }

    connections allowunconnected:
        // Connect each node to its fingers using explicit indices
        for i=0..numClients-1 {
            // Successor (first finger)
            client[i].out[(i+1)%numClients] --> DelayChannel --> client[(i+1)%numClients].in[i];
            client[i].gout[(i+1)%numClients] --> DelayChannel --> client[(i+1)%numClients].gin[i];
            
            // Second finger (i+2)
            client[i].out[(i+2)%numClients] --> DelayChannel --> client[(i+2)%numClients].in[i];
            client[i].gout[(i+2)%numClients] --> DelayChannel --> client[(i+2)%numClients].gin[i];
            
            // Third finger (i+4)
            client[i].out[(i+4)%numClients] --> DelayChannel --> client[(i+4)%numClients].in[i];
            client[i].gout[(i+4)%numClients] --> DelayChannel --> client[(i+4)%numClients].gin[i];
            
            // Fourth finger (i+8)
            client[i].out[(i+8)%numClients] --> DelayChannel --> client[(i+8)%numClients].in[i];
            client[i].gout[(i+8)%numClients] --> DelayChannel --> client[(i+8)%numClients].gin[i];
        }
}

