// Project identifier: 9504853406CBAC39EE89AA3AD238AA12CA262043

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <deque>
#include <vector>
#include <getopt.h>
#include <iostream>
#include "P2random.h"
using namespace std;

struct Zombies{
    string name;
    uint32_t distance;
    uint32_t speed;
    uint32_t health;
    uint32_t roundsActive;
    Zombies() : name(" "), distance(0), speed(0), health(0), roundsActive(0) {}
};
struct Comparator{ 
    bool operator()(const Zombies* a, const Zombies* b) const {
        uint32_t aETA = a->distance/a->speed;
        uint32_t bETA = b->distance/b->speed;
        if(aETA != bETA){
            return aETA > bETA;
        }else if(a->health != b->health){
            return a->health > b->health;
        }else {
            return a->name > b->name;
        }
    }
};
struct LeastActive{ 
    bool operator()(const Zombies& a, const Zombies& b) const {
        if(a.roundsActive != b.roundsActive){
            return a.roundsActive > b.roundsActive;
        }else {
            return a.name < b.name;
        }
    }
};//min heap
struct MostActive{ 
    bool operator()(const Zombies& a, const Zombies& b) const {
        if(a.roundsActive != b.roundsActive){
            return a.roundsActive < b.roundsActive;
        }else {
            return a.name < b.name;
        }
    }
};//max heap
struct MostActive2{ 
    bool operator()(const Zombies& a, const Zombies& b) const {
        if(a.roundsActive != b.roundsActive){
            return a.roundsActive < b.roundsActive;
        }else {
            return a.name > b.name;
        }
    }
};//max heap
struct LeastActive2{ 
    bool operator()(const Zombies& a, const Zombies& b) const {
        if(a.roundsActive != b.roundsActive){
            return a.roundsActive > b.roundsActive;
        }else {
            return a.name > b.name;
        }
    }
};
class ZombHolder{ 
    public:
    priority_queue<Zombies*, vector<Zombies*>, Comparator> zombQueue;
    deque<Zombies> masterList;
    void get_options(int argc, char** argv);
    void readHeader();
    void readRound();
    void playRound();
    void statsPrint();
    bool areDead = false;
    uint32_t roundNum = 0;
    uint32_t currentMedian = 0;
    Zombies killer;
    ZombHolder() : verboseMode(false), medianMode(false), statsMode(false), statsNum(0) {}
    uint32_t nextRound = 0; //next round in file
    private:
    bool verboseMode = false;
    bool medianMode = false;
    bool statsMode = false;
    int statsNum;
    uint32_t numKilled = 0;
    vector<string> firstDeaths;
    priority_queue<Zombies, vector<Zombies>, LeastActive> mostAct;
    priority_queue<Zombies, vector<Zombies>, MostActive> leastAct;
    deque<string> lastDeaths;
    uint32_t quiverCapacity = 0;
    priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>> highHalf;
    priority_queue<uint32_t> lowHalf;
};

// Read and process command line options.
void ZombHolder::get_options(int argc, char** argv){ 
    int option_index = 0, option = 0;
    
    // Don't display getopt error messages about options
    opterr = false;
    
    // use getopt to find command line options
    struct option longOpts[] = {{ "verbose", no_argument, nullptr, 'v'},
                                { "statistics", required_argument, nullptr, 's'},
                                { "median", no_argument, nullptr, 'm' },
                                { "help", no_argument, nullptr, 'h' },
                                { nullptr, 0, nullptr, '\0' }
                            };
    

    while ((option = getopt_long(argc, argv, "vs:mh", longOpts, &option_index)) != -1) {
        switch (option) {
            case 'v':
            verboseMode = true;
            break;
            case 's':
            statsMode = true;
            statsNum = atoi(optarg);
            break;
            case 'm':
            medianMode = true;
            break;
            case 'h':
            //print help message
            exit(0);
        }
    }
}

void ZombHolder::readHeader(){
    string junk;
    uint32_t randSeed, distance, speed, health;
    getline(cin, junk);
    cin>>junk>>quiverCapacity>>junk>>randSeed>>junk>>distance>>junk>>speed>>junk>>health>>junk;
    P2random::initialize(randSeed, distance, speed, health);
    getline(cin, junk);
    cin>>junk>>roundNum;
    nextRound = roundNum;
    roundNum = 0;
}

void ZombHolder::readRound(){
    string junk; Zombies Z; uint32_t zombNum;
    cin>>junk>>zombNum;
    for(uint32_t i = 0; i < zombNum; i++){
      	string name  = P2random::getNextZombieName();
        uint32_t distance = P2random::getNextZombieDistance();
        uint32_t speed    = P2random::getNextZombieSpeed();
        uint32_t health   = P2random::getNextZombieHealth();
        Z.name = name;
        Z.distance = distance;
        Z.speed = speed;
        Z.health = health;
        Z.roundsActive = 1;
        masterList.push_back(Z);
        Zombies* pointZ = &masterList.back();
        zombQueue.push(pointZ);
        if(verboseMode){
            cout << "Created: " << Z.name << " (distance: "<< Z.distance 
                << ", speed: " << Z.speed << ", health: "<< Z.health << ")\n"; 
        }
    }
    cin>>junk>>zombNum;
    for(uint32_t i = 0; i < zombNum; i++){
        cin>>Z.name>>junk>>Z.distance>>junk>>Z.speed>>junk>>Z.health;
        masterList.push_back(Z);
        Zombies* pointZ = &masterList.back();
        zombQueue.push(pointZ);
        if(verboseMode){
            cout << "Created: " << Z.name << " (distance: "<< Z.distance 
                << ", speed: " << Z.speed << ", health: "<< Z.health << ")\n"; 
        }
    }
    cin>>junk;
    cin>>junk>>nextRound;
    if(cin.fail()){
        nextRound = 0;
        return;
    }
}

void ZombHolder::playRound(){
    uint32_t quiver;
    quiver = quiverCapacity;
    ++roundNum;
    if(verboseMode){
        cout << "Round: " << roundNum << '\n';
    }
    for(size_t i = 0; i < masterList.size(); ++i){
        if(masterList[i].health != 0){
            masterList[i].distance -= min(masterList[i].distance, masterList[i].speed);
            masterList[i].roundsActive++;
            if(verboseMode){
                cout << "Moved: " << masterList[i].name << " (distance: "<< masterList[i].distance 
                    << ", speed: " << masterList[i].speed << ", health: "<< masterList[i].health << ")\n"; 
            }
            if(masterList[i].distance == 0 && areDead == false){
            killer = masterList[i];
            areDead = true;
            }
        }
        
    }
    if(areDead == true){
        cout<< "DEFEAT IN ROUND " << roundNum << "! "<< killer.name <<" ate your brains!\n";
        return;
    }
    if(nextRound == roundNum){
        readRound();
    }
    while(quiver != 0 && !zombQueue.empty()){
        quiver--;
        zombQueue.top()->health--;
        if(zombQueue.top()->health == 0){
            if(verboseMode == true){
                cout << "Destroyed: " << zombQueue.top()->name << " (distance: "<< zombQueue.top()->distance 
                << ", speed: " << zombQueue.top()->speed << ", health: "<< zombQueue.top()->health << ")\n";
            }
            if(zombQueue.size() == 1){
                killer = *zombQueue.top();
            }
            if(statsMode){
                ++numKilled;
                if(firstDeaths.size() < static_cast<size_t>(statsNum)){
                    firstDeaths.push_back(zombQueue.top()->name);
                }
                lastDeaths.push_front(zombQueue.top()->name);
                if(lastDeaths.size() > static_cast<size_t>(statsNum)){
                    lastDeaths.pop_back();
                }
            }
            if(medianMode){
                //check median currently 
                //add to correct heap
                //check rebalancing
                if(zombQueue.top()->roundsActive > currentMedian){
                    highHalf.push(zombQueue.top()->roundsActive);
                }else{
                    lowHalf.push(zombQueue.top()->roundsActive);
                }
                if(lowHalf.size() > (highHalf.size()+1)){
                    uint32_t temp = lowHalf.top();
                    highHalf.push(temp);
                    lowHalf.pop();
                }
                if(highHalf.size() > (lowHalf.size()+1)){
                    uint32_t temp = highHalf.top();
                    lowHalf.push(temp);
                    highHalf.pop();
                }
                if(highHalf.size() == lowHalf.size()){
                    currentMedian = (highHalf.top() + lowHalf.top())/2;
                }else if(highHalf.size() > lowHalf.size()){
                    currentMedian = highHalf.top();
                }else{
                    currentMedian = lowHalf.top();
                }

            }
            zombQueue.pop();
        }
    }
    if(medianMode){
        cout << "At the end of round " <<roundNum<< ", the median zombie lifetime is " << currentMedian << '\n';
    }
}

void ZombHolder::statsPrint(){
    if(statsMode){
        uint32_t count = 0;
        for(size_t i = 0; i < masterList.size(); ++i){
            if(masterList[i].health != 0){
                ++count;
            }
        }
        cout << "Zombies still active: "<< count <<"\n";
        cout << "First zombies killed:\n";
        for(size_t i = 0; i < firstDeaths.size(); ++i){
            cout << firstDeaths[i] << " " << i+1 << "\n";
        }
        cout << "Last zombies killed:\n";
        for(size_t i = 0; i < lastDeaths.size(); ++i){
            cout << lastDeaths[i] << " " << numKilled - static_cast<uint32_t>(i) << "\n";
        }

        cout << "Most active zombies:\n";
        for(size_t i = 0; i < masterList.size(); ++i){
            mostAct.push(masterList[i]);
            if(mostAct.size() > static_cast<size_t>(statsNum)){
                mostAct.pop();
            }
        }//min heap to get most active zombies by popping all the min ones
        size_t size = mostAct.size();
        priority_queue<Zombies, vector<Zombies>, MostActive2> resorted;
        for(size_t i = 0; i < size; ++i){
            resorted.push(mostAct.top());
            mostAct.pop();
        }
        size = resorted.size();
        for(size_t i = 0; i < size; ++i){
            cout << resorted.top().name << " " << resorted.top().roundsActive << '\n';
            resorted.pop();
        }
        cout << "Least active zombies:\n";
        for(size_t i = 0; i < masterList.size(); ++i){
            leastAct.push(masterList[i]);
            if(leastAct.size() > static_cast<size_t>(statsNum)){
                leastAct.pop();
            }
        }//max heap to get most least zombies by popping all the max ones
        size = leastAct.size();
        priority_queue<Zombies, vector<Zombies>, LeastActive2> resorted2;
        for(size_t i = 0; i < size; ++i){
            resorted2.push(leastAct.top());
            leastAct.pop();
        }
        size = resorted2.size();
        for(size_t i = 0; i < size; ++i){
            cout << resorted2.top().name << " " << resorted2.top().roundsActive << '\n';
            resorted2.pop();
        }
    }
}

int main(int argc, char** argv){
    ios_base::sync_with_stdio(false);
    ZombHolder simulation;
    simulation.get_options(argc, argv);
    simulation.readHeader();
    while(!simulation.areDead && (!simulation.zombQueue.empty() || simulation.nextRound != 0)){
        simulation.playRound();
    }
    if(!simulation.areDead){
        cout<<"VICTORY IN ROUND "<< simulation.roundNum <<"! "<<simulation.killer.name<<" was the last zombie.\n";
    }
    simulation.statsPrint();
    return 0;
}