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
        int numClients = default(9);
    
    types:
        channel DelayChannel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        client[numClients]: ClientNode {
            parameters:
                id = index;
                numClients = parent.numClients;
                taskSizes = "5,5";
        }

    connections allowunconnected:
        // Connect each node to its fingers using explicit indices
        client[0].out[1] --> DelayChannel --> client[1].in[0];
        client[0].gout[1] --> DelayChannel --> client[1].gin[0];
        client[0].out[2] --> DelayChannel --> client[2].in[0];
        client[0].gout[2] --> DelayChannel --> client[2].gin[0];
        client[0].out[4] --> DelayChannel --> client[4].in[0];
        client[0].gout[4] --> DelayChannel --> client[4].gin[0];
        client[0].out[8] --> DelayChannel --> client[8].in[0];
        client[0].gout[8] --> DelayChannel --> client[8].gin[0];
        client[1].out[2] --> DelayChannel --> client[2].in[1];
        client[1].gout[2] --> DelayChannel --> client[2].gin[1];
        client[1].out[3] --> DelayChannel --> client[3].in[1];
        client[1].gout[3] --> DelayChannel --> client[3].gin[1];
        client[1].out[5] --> DelayChannel --> client[5].in[1];
        client[1].gout[5] --> DelayChannel --> client[5].gin[1];
        client[1].out[0] --> DelayChannel --> client[0].in[1];
        client[1].gout[0] --> DelayChannel --> client[0].gin[1];
        client[2].out[3] --> DelayChannel --> client[3].in[2];
        client[2].gout[3] --> DelayChannel --> client[3].gin[2];
        client[2].out[4] --> DelayChannel --> client[4].in[2];
        client[2].gout[4] --> DelayChannel --> client[4].gin[2];
        client[2].out[6] --> DelayChannel --> client[6].in[2];
        client[2].gout[6] --> DelayChannel --> client[6].gin[2];
        client[2].out[1] --> DelayChannel --> client[1].in[2];
        client[2].gout[1] --> DelayChannel --> client[1].gin[2];
        client[3].out[4] --> DelayChannel --> client[4].in[3];
        client[3].gout[4] --> DelayChannel --> client[4].gin[3];
        client[3].out[5] --> DelayChannel --> client[5].in[3];
        client[3].gout[5] --> DelayChannel --> client[5].gin[3];
        client[3].out[7] --> DelayChannel --> client[7].in[3];
        client[3].gout[7] --> DelayChannel --> client[7].gin[3];
        client[3].out[2] --> DelayChannel --> client[2].in[3];
        client[3].gout[2] --> DelayChannel --> client[2].gin[3];
        client[4].out[5] --> DelayChannel --> client[5].in[4];
        client[4].gout[5] --> DelayChannel --> client[5].gin[4];
        client[4].out[6] --> DelayChannel --> client[6].in[4];
        client[4].gout[6] --> DelayChannel --> client[6].gin[4];
        client[4].out[8] --> DelayChannel --> client[8].in[4];
        client[4].gout[8] --> DelayChannel --> client[8].gin[4];
        client[4].out[3] --> DelayChannel --> client[3].in[4];
        client[4].gout[3] --> DelayChannel --> client[3].gin[4];
        client[5].out[6] --> DelayChannel --> client[6].in[5];
        client[5].gout[6] --> DelayChannel --> client[6].gin[5];
        client[5].out[7] --> DelayChannel --> client[7].in[5];
        client[5].gout[7] --> DelayChannel --> client[7].gin[5];
        client[5].out[0] --> DelayChannel --> client[0].in[5];
        client[5].gout[0] --> DelayChannel --> client[0].gin[5];
        client[5].out[4] --> DelayChannel --> client[4].in[5];
        client[5].gout[4] --> DelayChannel --> client[4].gin[5];
        client[6].out[7] --> DelayChannel --> client[7].in[6];
        client[6].gout[7] --> DelayChannel --> client[7].gin[6];
        client[6].out[8] --> DelayChannel --> client[8].in[6];
        client[6].gout[8] --> DelayChannel --> client[8].gin[6];
        client[6].out[1] --> DelayChannel --> client[1].in[6];
        client[6].gout[1] --> DelayChannel --> client[1].gin[6];
        client[6].out[5] --> DelayChannel --> client[5].in[6];
        client[6].gout[5] --> DelayChannel --> client[5].gin[6];
        client[7].out[8] --> DelayChannel --> client[8].in[7];
        client[7].gout[8] --> DelayChannel --> client[8].gin[7];
        client[7].out[0] --> DelayChannel --> client[0].in[7];
        client[7].gout[0] --> DelayChannel --> client[0].gin[7];
        client[7].out[2] --> DelayChannel --> client[2].in[7];
        client[7].gout[2] --> DelayChannel --> client[2].gin[7];
        client[7].out[6] --> DelayChannel --> client[6].in[7];
        client[7].gout[6] --> DelayChannel --> client[6].gin[7];
        client[8].out[0] --> DelayChannel --> client[0].in[8];
        client[8].gout[0] --> DelayChannel --> client[0].gin[8];
        client[8].out[1] --> DelayChannel --> client[1].in[8];
        client[8].gout[1] --> DelayChannel --> client[1].gin[8];
        client[8].out[3] --> DelayChannel --> client[3].in[8];
        client[8].gout[3] --> DelayChannel --> client[3].gin[8];
        client[8].out[7] --> DelayChannel --> client[7].in[8];
        client[8].gout[7] --> DelayChannel --> client[7].gin[8];
}
