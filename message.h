//B22CS081_B22CS030
#include <omnetpp.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <bits/stdc++.h>
#pragma once

class CsMsg : public omnetpp::cMessage {
public:
    int clientId = 0;
    int serverId = 0;
    int subtaskId = 0;
    std::vector<int > elements;
    int maxE = -1 ; // stores the values of maxE when returning ?? or should we create a new struct ??
    bool isResponse = false;  // to verify who send the message
    omnetpp::simtime_t currentTime;
};


// Add isRouting field to your message class
class PeerTaskMsg : public omnetpp::cMessage {
public:
    int sourceClientId;
    int destClientId;
    int subtaskId;
    std::vector<int> elements;
    int result;
    bool isResponse;
    omnetpp::simtime_t currentTime;
    bool isRouting = false;  // New field to indicate if message is being routed
};

// Make sure to add isRouting to gossipMessage class as well
class gossipMessage : public omnetpp ::cMessage {
public:
    int sourceClientId;
    int destClientId;
    std::vector<float> serverScores;
    omnetpp::simtime_t currentTime;
    bool isRouting = false;  // New field for routing
};
