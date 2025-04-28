// B22CS081_B22CS030
#include "message.h"
#include <fstream>
#include <cmath> // For log2 and ceil functions
#include <algorithm>
using namespace omnetpp;

class ClientNode : public cSimpleModule
{
private:
    std::ofstream logFile;

public:
    int numClients;
    std::vector<int> originalVector;
    std::vector<int> selectedPeerIds;
    std::map<int, std::map<int, int>> peerResponses; // map of subtaskId and peerId and result
    std::map<int, int> subtaskResult;
    int subTask = 0;
    int responses = 0;
    bool taskCompleted = false; // Renamed from scoresAlreadySent
    int currentTask = 0;
    std::vector<int> taskSizeList;
    std::vector<int> fingerTable;  // CHORD finger table


    virtual void initialize() override
    {
        // Open the log file
        logFile.open("output.txt", std::ios_base::app);
        if (!logFile.is_open())
            EV << "Error opening log file!" << "\n";

        // Get basic parameters
        numClients = par("numClients");

        // Initialize finger table for CHORD routing
        initializeFingerTable();

        // Extract the taskSizes array parameter
        taskSizeList.clear();
        std::string taskSizesStr = par("taskSizes").stringValue();
        std::stringstream ss(taskSizesStr);
        int size;
        while (ss >> size)
        {
            taskSizeList.push_back(size);
            if (ss.peek() == ',')
                ss.ignore();
        }

        if (taskSizeList.empty())
        {
            taskSizeList.push_back(5); // Default value if parsing fails
        }
        EV << "Printing the Sizes" << endl;
        for (int i = 0; i < taskSizeList.size(); i++)
        {
            EV << taskSizeList[i] << " ";
        }
        EV << endl;

        currentTask = 0;

        originalVector = generateRandomVector(taskSizeList[currentTask]);
        selectedPeerIds = selectPeers();

        sendSubTasks();
    }

    void initializeFingerTable() {
        int nodeId = getIndex();
        int numFingers = ceil(log2(numClients));
        fingerTable.resize(numFingers);
        
        for (int i = 0; i < numFingers; i++) {
            // For each finger i, calculate (n + 2^i) % numClients
            fingerTable[i] = (nodeId + (1 << i)) % numClients;
            
            if (logFile.is_open())
                logFile << "Client " << nodeId << " finger[" << i << "] points to " << fingerTable[i] << endl;
            
            EV << "Client " << nodeId << " finger[" << i << "] points to " << fingerTable[i] << endl;
        }
    }

    int findNextHop(int targetId) {
        int nodeId = getIndex();
        
        // If the target is the current node, return self
        if (targetId == nodeId) return nodeId;
        
        // If target is between current node and successor, route to successor
        int successor = (nodeId + 1) % numClients;
        if (isInRange(targetId, nodeId, successor)) return successor;
        
        // Otherwise, find the closest preceding finger
        for (int i = fingerTable.size() - 1; i >= 0; i--) {
            int fingerId = fingerTable[i];
            if (isInRange(fingerId, nodeId, targetId)) {
                return fingerId;
            }
        }
        
        // Fallback to immediate successor
        return successor;
    }

    bool isInRange(int id, int start, int end) {
        // Check if id is in the range (start, end)
        // Handle wrap-around case in the ring
        if (start < end)
            return (id > start && id <= end);
        else  // wrap around case
            return (id > start || id <= end);
    }

    virtual void finish() override
    {
        logFile.close();
    }

    virtual void handleMessage(cMessage *msg) override
    {
        // Check if this is a task message from another peer
        PeerTaskMsg *peerMsg = dynamic_cast<PeerTaskMsg *>(msg);
        if (peerMsg)
        {
            if (peerMsg->isResponse)
            {
                // This is a response to a task we sent
                handlePeerResponse(peerMsg);
            }
            else
            {
                // This is a new task to process
                processPeerTask(peerMsg);
            }
            return;
        }

        // Check if it's time to start a new task
        if (strcmp(msg->getName(), "startNewTask") == 0)
        {
            startNewTask();
            delete msg;
            return;
        }

        // Schedule the next task if we've finished current one
        if (taskCompleted && currentTask < int(taskSizeList.size()) - 1)
        {
            if (logFile.is_open())
                logFile << "Client " << getIndex() << " finished current task, ready for next" << endl;

            EV << "Client " << getIndex() << " finished current task, ready for next" << endl;

            // Schedule a new task after a delay
            cMessage *startNewTaskMsg = new cMessage("startNewTask");
            scheduleAt(simTime() + 1.0, startNewTaskMsg);
        }
    }

    void handlePeerResponse(PeerTaskMsg *msg)
    {
        // If message is being routed and not for us, forward it
        if (msg->isRouting && msg->destClientId != getIndex()) {
            int nextHop = findNextHop(msg->destClientId);
            
            // Create a copy and forward
            PeerTaskMsg *forwardMsg = new PeerTaskMsg(*msg);
            send(forwardMsg, "out", nextHop);
            
            if (logFile.is_open())
                logFile << simTime() << " Client " << getIndex()
                        << " : Forwarding response for subtask " << msg->subtaskId 
                        << " from Client " << msg->sourceClientId 
                        << " to Client " << msg->destClientId << endl;
            
            delete msg;
            return;
        }

        peerResponses[msg->subtaskId][msg->sourceClientId] = msg->result;

        if (logFile.is_open())
            logFile << msg->currentTime << " Client " << getIndex() << " : Received result "
                    << msg->result << " from Client " << msg->sourceClientId
                    << " for subtask " << msg->subtaskId << endl;

        EV << msg->currentTime << " Client " << getIndex() << " : Received result "
           << msg->result << " from Client " << msg->sourceClientId
           << " for subtask " << msg->subtaskId << endl;

        responses++;

        // Check if we've received all responses
        if (responses == subTask && !taskCompleted)
        {
            processResponses();
            taskCompleted = true;
            
            // Schedule the next task
            cMessage *newTaskMsg = new cMessage("startNewTask");
            scheduleAt(simTime() + 1.0, newTaskMsg);
        }

        delete msg;
    }

    void processPeerTask(PeerTaskMsg *msg)
    {
        // If this message is being routed and not meant for us, forward it
        if (msg->isRouting && msg->destClientId != getIndex()) {
            // Find next hop for the destination
            int nextHop = findNextHop(msg->destClientId);
            
            // Forward the message
            PeerTaskMsg *forwardMsg = new PeerTaskMsg(*msg);  // Create a copy
            send(forwardMsg, "out", nextHop);
            
            if (logFile.is_open())
                logFile << simTime() << " Client " << getIndex()
                        << " : Forwarding subtask " << msg->subtaskId 
                        << " from Client " << msg->sourceClientId 
                        << " to Client " << msg->destClientId
                        << " via hop " << nextHop << endl;
            
            EV << "Client " << getIndex() << " forwarding message from "
               << msg->sourceClientId << " to " << msg->destClientId
               << " via hop " << nextHop << endl;
               
            delete msg;
            return;
        }
        
        // Process the task (find max element)
        const std::vector<int> &elements = msg->elements;
        int result = *std::max_element(elements.begin(), elements.end());
        
        // Create response message
        PeerTaskMsg *response = new PeerTaskMsg();
        response->sourceClientId = getIndex();
        response->destClientId = msg->sourceClientId;
        response->subtaskId = msg->subtaskId;
        response->result = result;
        response->isResponse = true;
        response->currentTime = simTime();
        
        if (logFile.is_open())
            logFile << response->currentTime << " Client " << getIndex()
                    << " : Processing subtask from Client " << msg->sourceClientId
                    << ", sending result " << result << endl;
        
        EV << response->currentTime << " Client " << getIndex()
           << " : Processing subtask from Client " << msg->sourceClientId
           << ", sending result " << result << endl;
        
        // Use CHORD routing for the response as well
        int nextHop = findNextHop(msg->sourceClientId);
        if (nextHop != msg->sourceClientId) {
            response->isRouting = true;
        }
        
        // Send response back to the requesting peer via next hop
        send(response, "out", nextHop);
        
        delete msg;
    }

    std::vector<int> selectPeers()
    {
        std::set<int> peers;
        while (peers.size() < (numClients) / 2)
        {
            int value = std::rand() % numClients;
            if (value != getIndex())
            { // Don't select self
                peers.insert(value);
            }
        }
        std::vector<int> selectedPeers(peers.begin(), peers.end());
        return selectedPeers;
    }

    std::vector<int> generateRandomVector(int size)
    {
        std::vector<int> result;
        for (int i = 0; i < size; i++)
        {
            int value = 1 + std::rand() % 100;
            result.push_back(value);
        }
        return result;
    }


    void sendSubTasks()
    {
        EV << "Client " << getIndex() << " sending subtasks to peers" << endl;

        int vectorSize = originalVector.size();
        int numSubtasks = std::min(vectorSize / 2, numClients); // Ensure k/x ≥ 2
        subTask = numSubtasks;                                  // Store the number of subtasks for later

        // Calculate elements per subtask, ensuring at least 2 elements per subtask
        int elementsPerSubtask = vectorSize / numSubtasks;
        if (elementsPerSubtask < 2)
        {
            elementsPerSubtask = 2;
            numSubtasks = vectorSize / elementsPerSubtask; // Recalculate number of subtasks
        }

        if (logFile.is_open())
            logFile << "Client " << getIndex() << " dividing task into " << numSubtasks
                    << " subtasks with " << elementsPerSubtask << " elements each" << endl;

        EV << "Client " << getIndex() << " dividing task into " << numSubtasks
           << " subtasks with " << elementsPerSubtask << " elements each" << endl;

        // Generate random subtask IDs
        std::vector<int> subtaskIds(numSubtasks);
        for (int i = 0; i < numSubtasks; i++)
        {
            subtaskIds[i] = std::rand() % 1000; // Large range to minimize collisions
        }

        // Create a map to track workload per client
        std::map<int, int> clientWorkload;
        for (int i = 0; i < numClients; i++)
        {
            clientWorkload[i] = 0;
        }

        // Process each subtask -  send each subtask to only one peer
        for (int i = 0; i < numSubtasks; i++)
        {
            // Calculate start and end indices for this subtask
            int startIdx = i * elementsPerSubtask;
            int endIdx = (i == numSubtasks - 1) ? vectorSize : (i + 1) * elementsPerSubtask;

            // Extract elements for this subtask
            std::vector<int> subtaskElements(originalVector.begin() + startIdx,
                                             originalVector.begin() + endIdx);

            // Use the random subtask ID to determine target client
            int subtaskId = subtaskIds[i];
            
            // Selecting only one peer per subtask - 
            int targetClientId;
            if (getIndex() == i % numClients) {
                // Avoid sending to self, pick next client
                targetClientId = (getIndex() + 1) % numClients;
            } else {
                targetClientId = i % numClients; 
            }

            // Increment workload for this client
            clientWorkload[targetClientId]++;

            // Send the subtask using CHORD routing
            sendSubTask(targetClientId, subtaskId, subtaskElements);
        }

        // Log the workload distribution
        EV << "Workload distribution: ";
        for (int i = 0; i < numClients; i++)
        {
            EV << "Client " << i << ": " << clientWorkload[i] << " tasks, ";
        }
        EV << endl;
    }

    void sendSubTask(int targetClientId, int subtaskId, std::vector<int>& subtaskElements) {
        // If target is self, process locally
        if (targetClientId == getIndex()) {
            int result = *std::max_element(subtaskElements.begin(), subtaskElements.end());
            peerResponses[subtaskId][getIndex()] = result;
            
            if (logFile.is_open())
                logFile << simTime() << " Client " << getIndex()
                        << " : Processing subtask " << subtaskId << " locally, result: " << result << endl;
            
            EV << simTime() << " Client " << getIndex()
               << " : Processing subtask " << subtaskId << " locally, result: " << result << endl;
            
            responses++;
            return;
        }
        
        // Find next hop using CHORD routing
        int nextHop = findNextHop(targetClientId);
        
        // Create the message
        PeerTaskMsg *msgToPeer = new PeerTaskMsg();
        msgToPeer->sourceClientId = getIndex();
        msgToPeer->destClientId = targetClientId;  // Final destination
        msgToPeer->subtaskId = subtaskId;
        msgToPeer->elements = subtaskElements;
        msgToPeer->isResponse = false;
        msgToPeer->currentTime = simTime();
        msgToPeer->isRouting = nextHop != targetClientId;  // Set routing flag if not direct
        
        // Send message to next hop
        send(msgToPeer, "out", nextHop);
        
        if (logFile.is_open())
            logFile << msgToPeer->currentTime << " Client " << getIndex()
                    << " : Sent subtask " << subtaskId << " to Client " << targetClientId 
                    << " via hop " << nextHop << endl;
        
        EV << "Client " << getIndex() << " sent subtask " << subtaskId
           << " with " << subtaskElements.size() << " elements to client " << targetClientId
           << " via hop " << nextHop << endl;
    }

    void processResponses()
    {
        if (logFile.is_open())
            logFile << "Client " << getIndex() << " received responses from peers" << endl;

        EV << "Client " << getIndex() << " received responses from peers" << endl;

        int finalResult = 0;

        // Process each subtask response to find the maximum value
        for (auto &subtaskEntry : peerResponses)
        {
            int subtaskId = subtaskEntry.first;
            auto &responses = subtaskEntry.second;

            // Since we only have one response per subtask, directly use that
            int result = responses.begin()->second;

            // Update the result for this subtask
            subtaskResult[subtaskId] = result;
            
            // Update the maximum overall result
            if (result > finalResult) {
                finalResult = result;
            }
        }

        if (logFile.is_open()) {
            logFile << "Client " << getIndex() << " received responses from peers" << endl;
            logFile << "Client " << getIndex() << " final result: " << finalResult << endl;
        }

        EV << "Client " << getIndex() << " final result: " << finalResult << endl;
    }

    void startNewTask()
    {
        if (currentTask < taskSizeList.size() - 1)
        {
            currentTask++;
            responses = 0;
            taskCompleted = false;
            peerResponses.clear();
            subtaskResult.clear();

            originalVector = generateRandomVector(taskSizeList[currentTask]);
            
            // Select peers for this task without using peer scores
            selectedPeerIds = selectPeers(); 
            
            sendSubTasks();
        }
    }
};

Define_Module(ClientNode);