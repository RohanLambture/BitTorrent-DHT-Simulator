//B22CS081_B22CS030
#include "message.h"
using namespace omnetpp;

class ServerNode : public cSimpleModule {
private:
    std::ofstream logFile;
public:
    virtual void initialize() override {
        // Open the log file (this file will be written to in addition to the EV output)
        logFile.open("output.txt", std::ios_base::app);
        if (!logFile.is_open())
            EV << "Error opening log file!" << "\n";
    }

    virtual void finish() override {
        logFile.close();
    }

    virtual void handleMessage(cMessage *msg) override {

        
        //why needed ?????
        CsMsg *csMsg = check_and_cast<CsMsg *>(msg);

        // finding the maximum element in the vector which came from msg
        const std::vector<int>& elements = csMsg->elements;
        int value = 1 + std::rand() % 100;

        int answer ;
        if(value >75){
            answer = *std::min_element(elements.begin(), elements.end());
        }else {
            answer = *std::max_element(elements.begin(), elements.end());
        }
        

        // generaeting a response message
        CsMsg *response = new CsMsg();
        response->isResponse = true;
        response->maxE = answer;
        response->serverId = getIndex();
        response->clientId = csMsg->clientId;
        response->subtaskId = csMsg->subtaskId;
        response->currentTime = simTime();
        if (logFile.is_open())
            logFile << response->currentTime << " Server " << response->serverId << " : Sending " <<  response->maxE << " to client " << response->clientId  << "for subtask " << response->subtaskId << endl;
        EV << response->currentTime << " Server " << response->serverId << " : Sending " <<  response->maxE << " to client " << response->clientId  << "for subtask " << response->subtaskId << endl;  
        send(response, "out", csMsg->clientId);

        delete csMsg; //delete the original incoming message
    }
};

Define_Module(ServerNode);
