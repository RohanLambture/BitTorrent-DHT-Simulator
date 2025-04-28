// B22CS081_B22CS030
#include "message.h"
#include <fstream>
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
    std::vector<float> peerScores;
    std::map<int, int> subtaskResult;
    int subTask = 0;
    int responses = 0;
    bool scoresAlreadySent = false;
    int currentTask = 0;
    std::vector<int> taskSizeList;

    // virtual void initialize() override
    // {
    //     // Open the log file
    //     logFile.open("output.txt", std::ios_base::app);
    //     if (!logFile.is_open())
    //         EV << "Error opening log file!" << "\n";

    //     // Get basic parameters
    //     numClients = par("numClients");

    //     // Initialize peer score vector
    //     peerScores.resize(numClients, 0);

    //     // Extract the taskSizes array parameter
    //     taskSizeList.clear();
    //     taskSizeList.push_back(5);
    //     currentTask = 0;

    //     originalVector = generateRandomVector(taskSizeList[currentTask]);
    //     selectedPeerIds = selectPeers();

    //     sendSubTasks();
    // }

    virtual void initialize() override
    {
        // Open the log file
        logFile.open("output.txt", std::ios_base::app);
        if (!logFile.is_open())
            EV << "Error opening log file!" << "\n";

        // Get basic parameters
        numClients = par("numClients");

        // Initialize peer score vector
        peerScores.resize(numClients, 0);

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

    virtual void finish() override
    {
        logFile.close();
    }

    // virtual void handleMessage(cMessage *msg) override
    // {
    //     // Check if this is a task message from another peer
    //     PeerTaskMsg *peerMsg = dynamic_cast<PeerTaskMsg *>(msg);
    //     if (peerMsg)
    //     {
    //         if (peerMsg->isResponse)
    //         {
    //             // This is a response to a task we sent
    //             handlePeerResponse(peerMsg);
    //         }
    //         else
    //         {
    //             // This is a new task to process
    //             processPeerTask(peerMsg);
    //         }
    //         return;
    //     }

    //     // Check if this is a gossip message
    //     gossipMessage *gossipMsg = dynamic_cast<gossipMessage *>(msg);
    //     if (gossipMsg)
    //     {
    //         handleGossipMessage(gossipMsg);
    //         return;
    //     }

    //     if (scoresAlreadySent && currentTask < int(taskSizeList.size()) - 1)
    //     {
    //         if (logFile.is_open())
    //             logFile << "Client " << getIndex() << " finished current task, ready for next" << endl;

    //         EV << "Client " << getIndex() << " finished current task, ready for next" << endl;

    //         // Schedule a new task after a delay
    //         cMessage *startNewTaskMsg = new cMessage("startNewTask");
    //         scheduleAt(simTime() + 1.0, startNewTaskMsg);
    //     }

    //     // Add to handleMessage(), at the beginning
    //     if (strcmp(msg->getName(), "startNewTask") == 0)
    //     {
    //         startNewTask();
    //         delete msg;
    //         return;
    //     }
    // }

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

        // Check if this is a gossip message
        gossipMessage *gossipMsg = dynamic_cast<gossipMessage *>(msg);
        if (gossipMsg)
        {
            handleGossipMessage(gossipMsg);
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
        if (scoresAlreadySent && currentTask < int(taskSizeList.size()) - 1)
        {
            if (logFile.is_open())
                logFile << "Client " << getIndex() << " finished current task, ready for next" << endl;

            EV << "Client " << getIndex() << " finished current task, ready for next" << endl;

            // Schedule a new task after a delay
            cMessage *startNewTaskMsg = new cMessage("startNewTask");
            scheduleAt(simTime() + 1.0, startNewTaskMsg);
        }
    }

    // void handlePeerResponse(PeerTaskMsg *msg)
    // {
    //     peerResponses[msg->subtaskId][msg->sourceClientId] = msg->result;

    //     if (logFile.is_open())
    //         logFile << msg->currentTime << " Client " << getIndex() << " : Received result "
    //                 << msg->result << " from Client " << msg->sourceClientId
    //                 << " for subtask " << msg->subtaskId << endl;

    //     EV << msg->currentTime << " Client " << getIndex() << " : Received result "
    //        << msg->result << " from Client " << msg->sourceClientId
    //        << " for subtask " << msg->subtaskId << endl;

    //     responses++;

    //     // Check if we've received all responses
    //     if (responses == subTask * selectedPeerIds.size() && !scoresAlreadySent)
    //     {
    //         processResponses();
    //         sendScores();
    //         scoresAlreadySent = true;
    //     }

    //     delete msg;
    // }
    void handlePeerResponse(PeerTaskMsg *msg)
    {
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
        if (responses == subTask && !scoresAlreadySent)
        {
            processResponses();
            sendScores();
            scoresAlreadySent = true;
        }

        delete msg;
    }
    void processPeerTask(PeerTaskMsg *msg)
    {
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

        // Send response back to the requesting peer
        send(response, "out", msg->sourceClientId);

        delete msg;
    }

    void handleGossipMessage(gossipMessage *msg)
    {
        for (int i = 0; i < msg->serverScores.size() && i < peerScores.size(); i++)
        {
            peerScores[i] += msg->serverScores[i];
        }

        if (logFile.is_open())
            logFile << msg->currentTime << " Client " << getIndex()
                    << " : Received gossip message from client " << msg->sourceClientId << endl;

        EV << "Updated peer scores based on gossip from client " << msg->sourceClientId << endl;

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

    // void sendSubTasks()
    // {
    //     EV << "Client " << getIndex() << " sending subtasks to peers" << endl;

    //     int vectorSize = originalVector.size();

    //     // Calculate the number of subtasks (pairs of elements)
    //     if (vectorSize % 2 == 0)
    //     {
    //         subTask = vectorSize / 2;

    //         // Process pairs of elements
    //         for (int i = 0; i < vectorSize; i += 2)
    //         {
    //             std::vector<int> pair = {originalVector[i], originalVector[i + 1]};
    //             int subtaskId = i / 2;

    //             for (int j = 0; j < selectedPeerIds.size(); j++)
    //             {
    //                 PeerTaskMsg *msgToPeer = new PeerTaskMsg();
    //                 msgToPeer->sourceClientId = getIndex();
    //                 msgToPeer->destClientId = selectedPeerIds[j];
    //                 msgToPeer->subtaskId = subtaskId;
    //                 msgToPeer->elements = pair;
    //                 msgToPeer->isResponse = false;
    //                 msgToPeer->currentTime = simTime();

    //                 send(msgToPeer, "out", selectedPeerIds[j]);

    //                 if (logFile.is_open())
    //                     logFile << msgToPeer->currentTime << " Client " << getIndex()
    //                             << " : Sent subtask " << subtaskId << " to Client "
    //                             << selectedPeerIds[j] << endl;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         // For odd-sized vectors, handle pairs first
    //         int pairsCount = (vectorSize - 3) / 2;
    //         subTask = pairsCount + 1; // Regular pairs + the final triplet as one subtask

    //         // Process regular pairs
    //         for (int i = 0; i < 2 * pairsCount; i += 2)
    //         {
    //             std::vector<int> pair = {originalVector[i], originalVector[i + 1]};
    //             int subtaskId = i / 2;

    //             for (int j = 0; j < selectedPeerIds.size(); j++)
    //             {
    //                 PeerTaskMsg *msgToPeer = new PeerTaskMsg();
    //                 msgToPeer->sourceClientId = getIndex();
    //                 msgToPeer->destClientId = selectedPeerIds[j];
    //                 msgToPeer->subtaskId = subtaskId;
    //                 msgToPeer->elements = pair;
    //                 msgToPeer->isResponse = false;
    //                 msgToPeer->currentTime = simTime();

    //                 send(msgToPeer, "out", selectedPeerIds[j]);
    //             }
    //         }

    //         // Handle the last three elements as a single subtask
    //         int lastSubtaskId = pairsCount;
    //         std::vector<int> lastGroup = {
    //             originalVector[vectorSize - 3],
    //             originalVector[vectorSize - 2],
    //             originalVector[vectorSize - 1]};

    //         for (int j = 0; j < selectedPeerIds.size(); j++)
    //         {
    //             PeerTaskMsg *msgToPeer = new PeerTaskMsg();
    //             msgToPeer->sourceClientId = getIndex();
    //             msgToPeer->destClientId = selectedPeerIds[j];
    //             msgToPeer->subtaskId = lastSubtaskId;
    //             msgToPeer->elements = lastGroup;
    //             msgToPeer->isResponse = false;
    //             msgToPeer->currentTime = simTime();

    //             send(msgToPeer, "out", selectedPeerIds[j]);
    //         }
    //     }
    // }

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

        // Process each subtask
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
            int targetClientId = subtaskId % numClients;

            // Increment workload for this client
            clientWorkload[targetClientId]++;

            // Don't send to self - create a local subtask result instead
            if (targetClientId == getIndex())
            {
                // Process locally
                int result = *std::max_element(subtaskElements.begin(), subtaskElements.end());

                // Store the result directly
                peerResponses[subtaskId][getIndex()] = result;

                if (logFile.is_open())
                    logFile << simTime() << " Client " << getIndex()
                            << " : Processing subtask " << subtaskId << " locally, result: " << result << endl;

                EV << simTime() << " Client " << getIndex()
                   << " : Processing subtask " << subtaskId << " locally, result: " << result << endl;

                // Count this as a response
                responses++;
            }
            else
            {
                // Send to the target client
                PeerTaskMsg *msgToPeer = new PeerTaskMsg();
                msgToPeer->sourceClientId = getIndex();
                msgToPeer->destClientId = targetClientId;
                msgToPeer->subtaskId = subtaskId;
                msgToPeer->elements = subtaskElements;
                msgToPeer->isResponse = false;
                msgToPeer->currentTime = simTime();

                send(msgToPeer, "out", targetClientId);

                if (logFile.is_open())
                    logFile << msgToPeer->currentTime << " Client " << getIndex()
                            << " : Sent subtask " << subtaskId << " to Client " << targetClientId << endl;

                EV << "Client " << getIndex() << " sent subtask " << subtaskId
                   << " with " << subtaskElements.size() << " elements to client " << targetClientId << endl;
            }
        }

        // Log the workload distribution
        EV << "Workload distribution: ";
        for (int i = 0; i < numClients; i++)
        {
            EV << "Client " << i << ": " << clientWorkload[i] << " tasks, ";
        }
        EV << endl;

        // Check if all tasks were processed locally
        if (responses == numSubtasks)
        {
            processResponses();
            sendScores();
            scoresAlreadySent = true;
        }
    }

    void processResponses()
    {
        std::vector<float> clientScores(numClients, 0);

        for (auto &it : peerResponses)
        {
            int subtaskId = it.first;
            int maxCount = 0, maxResponse = -1;

            std::map<int, int> responses;
            for (auto &it2 : it.second)
            {
                int responseElement = it2.second;
                responses[responseElement]++;
                if (responses[responseElement] > maxCount)
                {
                    maxCount = responses[responseElement];
                    maxResponse = responseElement;
                }
            }

            // appending the maxResponse to the subtaskResult
            subtaskResult[subtaskId] = maxResponse;

            // updating the client scores
            for (auto &it2 : it.second)
            {
                if (it2.second == maxResponse)
                {
                    clientScores[it2.first]++;
                }
            }
        }

        if (logFile.is_open())
        {
            logFile << "Client " << getIndex() << " received responses from peers" << endl;
            logFile << "Client " << getIndex() << " peer scores: ";
        }

        EV << "Client " << getIndex() << " received responses from peers" << endl;
        EV << "Client " << getIndex() << " peer scores: ";

        for (int i = 0; i < clientScores.size(); i++)
        {
            this->peerScores[i] = float(clientScores[i]) / subTask;

            if (logFile.is_open())
                logFile << this->peerScores[i] << ", ";

            EV << peerScores[i] << ", ";
        }

        if (logFile.is_open())
            logFile << endl;

        EV << endl;

        // Calculate final result
        int finalResult = -1;
        for (auto &it : subtaskResult)
        {
            if (finalResult < it.second)
            {
                finalResult = it.second;
            }
        }

        if (logFile.is_open())
            logFile << "Client " << getIndex() << " final result: " << finalResult << endl;

        EV << "Client " << getIndex() << " final result: " << finalResult << endl;
    }

    void sendScores()
    {
        if (logFile.is_open())
            logFile << "Client " << getIndex() << " sending scores to other clients" << endl;

        EV << "Client " << getIndex() << " sending scores to other clients" << endl;

        for (int i = 0; i < numClients; i++)
        {
            if (i == getIndex())
                continue;

            gossipMessage *msgToClient = new gossipMessage();
            msgToClient->sourceClientId = getIndex();
            msgToClient->destClientId = i;
            msgToClient->serverScores = this->peerScores; // Reusing the serverScores field
            msgToClient->currentTime = simTime();

            send(msgToClient, "gout", i);

            if (logFile.is_open())
                logFile << "Client " << getIndex() << " sent peer scores to client " << i << endl;

            EV << "Sent peer scores from client " << getIndex() << " to client " << i << endl;
        }
    }

    void startNewTask()
    {
        responses = 0;
        subTask = 0;
        scoresAlreadySent = false;
        peerResponses.clear();
        subtaskResult.clear();

        if (logFile.is_open())
            logFile << "Client " << getIndex() << " is starting a new task" << endl;

        EV << "Client " << getIndex() << " is starting a new task" << endl;

        currentTask = currentTask + 1;
        // Generate a vector with enough elements to ensure at least 2 per subtask
        int taskSize = taskSizeList[currentTask];
        // Ensure size is large enough to have at least 2 elements per client
        if (taskSize < numClients * 2)
        {
            taskSize = numClients * 2;
        }
        originalVector = generateRandomVector(taskSize);

        EV << "Client " << getIndex() << " generated task of size " << originalVector.size() << endl;

        sendSubTasks();
    }

    // void startNewTask()
    // {
    //     responses = 0;
    //     subTask = 0;
    //     scoresAlreadySent = false;
    //     peerResponses.clear();
    //     subtaskResult.clear();

    //     selectedPeerIds = selectTopPeers();

    //     if (logFile.is_open())
    //         logFile << "Client " << getIndex() << " is starting a new task" << endl;

    //     EV << "Client " << getIndex() << " is starting a new task" << endl;

    //     for (int id : selectedPeerIds)
    //     {
    //         if (logFile.is_open())
    //             logFile << "Client " << getIndex() << " selected peer " << id << " ";

    //         EV << id << " ";
    //     }

    //     if (logFile.is_open())
    //         logFile << endl;

    //     EV << endl;

    //     currentTask = currentTask + 1;
    //     originalVector = generateRandomVector(taskSizeList[currentTask]);
    //     sendSubTasks();
    // }

    std::vector<int> selectTopPeers()
    {
        std::vector<std::pair<float, int>> peerRanking;
        for (int i = 0; i < peerScores.size(); i++)
        {
            if (i != getIndex())
            { // Don't include self
                peerRanking.push_back({peerScores[i], i});
            }
        }

        std::sort(peerRanking.begin(), peerRanking.end(),
                  [](const std::pair<float, int> &a, const std::pair<float, int> &b)
                  {
                      return a.first > b.first;
                  });

        std::vector<int> topPeers;
        int peersToSelect = (numClients) / 2;

        for (int i = 0; i < peerRanking.size() && i < peersToSelect; i++)
        {
            topPeers.push_back(peerRanking[i].second);
        }

        return topPeers;
    }
};

Define_Module(ClientNode);
