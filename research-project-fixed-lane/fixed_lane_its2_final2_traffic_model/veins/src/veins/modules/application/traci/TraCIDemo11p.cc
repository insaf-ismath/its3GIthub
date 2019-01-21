//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/TraCIDemo11p.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>

using namespace std;

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t TraCIDemo11p::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(TraCIDemo11p);

void TraCIDemo11p::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        mobility        = TraCIMobilityAccess().get(getParentModule());
        traci           = mobility->getCommandInterface();
        traciVehicle    = mobility->getVehicleCommandInterface();
        traciLane       = mobility->getLaneCommandInterface();
        annotations     = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
        findHost()->subscribe(parkingStateChangedSignal, this);

        traciVehicle->setSpeedMode(23);

        isEV = strcmp(traciVehicle->nodeId.c_str(),"vip") == 0;
        if(isEV){
            std::cout<<"Node["<<myId<<"] : isEV : lane = " << traciVehicle->getLaneId() << endl;
        }
        lane = stoi(traciVehicle->getLaneId());

        changeLane      = new cMessage("changeLane");
        beaconInterval  = par("beaconInterval").doubleValue();


        switch(EVStrategyUsed){
            case EV_BEST_LANE:
                if(isEV){
                    utilReCalcInterval  = par("utilReCalcInterval");
                    maxCarLengths   = trRegion/URegion;
                    trRegion        = 100; //0.01
                    utilFactor[0]   = 0;//          = {0.0,0.0};utilFactor[0] = 0;
                    utilFactor[1]   = 0;
                    minSpeed[0]     = traciLane->getMaxSpeed();
                    minSpeed[1]     = traciLane->getMaxSpeed();
                    cumSpeed[0]     = 0;
                    cumSpeed[1]     = 0;
                    vehicleCounter[0] = 0;
                    vehicleCounter[1] = 0;
                    w[0] = 0.4;
                    w[1] = 0.4;
                    w[2] = 0.2;

                    utilRecalcTimer     = new cMessage("utilRecalcTimer");
                    scheduleAt(simTime() + utilReCalcInterval, utilRecalcTimer);
                    std::cout<<"Node["<<myId<<"] : isEV : lane = " << lane << endl;
                }
                break;
            case EV_FIXED_LANE:
                bpd = par("bpdis");
                if(isEV){
                    std::cout<<"Node["<<myId<<"] : bpd = " << bpd << endl;
                    std::cout<<"Node["<<myId<<"] : isEV : lane = " << traciVehicle->getLaneId() << endl;
                }
                break;
        }

    }

}

void TraCIDemo11p::handleSelfMsg(cMessage *msg)
{
    switch(EVStrategyUsed){
        case EV_FIXED_LANE:
            handleSelfMsgFixedLane(msg);
            break;
        case EV_BEST_LANE:
            handleSelfMsgBestLane(msg);
            break;
    }
}

void TraCIDemo11p::handleSelfMsgFixedLane(cMessage *msg)
{
    if(msg == sendBeaconEvt && isEV){
        WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
        sendWSM(wsm);
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }else if(msg == changeLane){
        EV <<  "Change Lane called" << endl ;
        if(!dontChange){
            if(traciVehicle->getLaneIndex() == 0){
                traciVehicle->changeLane(1,50000);
                changed = true;
                lane = 1;
            }else if(traciVehicle->getLaneIndex() == 1){
                traciVehicle->changeLane(0,50000);
                changed = false;
                lane = 0;
            }
            std::cout<<"Node["<<myId<<"] : lane change : newLane = " << lane << endl;
        }else{
            scheduleChangeLane();
        }
    }
}

void TraCIDemo11p::handleSelfMsgBestLane(cMessage *msg)
{
    if(msg == sendBeaconEvt && isEV){
        std::cout<< "EV : Beacon sending: "<<endl;
        WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
        sendWSM(wsm);
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);

    }else if(msg == utilRecalcTimer){
        if(!changeLane->isScheduled()){
            utilFactor[0] = calculateUtility(0);
            utilFactor[1] = calculateUtility(1);

            if(utilFactor[abs(lane-1)] > utilFactor[lane]  ){
                WaveShortMessage* wsm = prepareWSM("want_change", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
                sendWSM(wsm);
                scheduleAt(simTime() + 2, changeLane);
            }
        }
        clearUtilMemory();
        scheduleAt(simTime() + utilReCalcInterval, utilRecalcTimer);
    }else if(msg == changeLane){
        EV <<  "Change Lane called" << endl ;
        if(!dontChange){
            if(traciVehicle->getLaneIndex() == 0){
                traciVehicle->changeLane(1,50000);
                changed = true;
                lane = 1;
            }else if(traciVehicle->getLaneIndex() == 1){
                traciVehicle->changeLane(0,50000);
                changed = false;
                lane = 0;
            }
            clearUtilMemory();
            std::cout<<"EV : lane change : newLane = " << lane << endl;
        }else{
            scheduleChangeLane();
        }
    }

}

double TraCIDemo11p::calculateUtility(int laneID){

    double maxSpeed = traciLane->getMaxSpeed(); // = max_speed_lane[lane_id];   //traciLane->getMaxSpeed()
    int vCount      = vehicleCounter[laneID];
    double avgSpeed;
    double freeSpace = maxCarLengths * utilReCalcInterval / beaconInterval;

    if(vCount == 0) avgSpeed = maxSpeed;
    else            avgSpeed = cumSpeed[laneID]/vehicleCounter[laneID];

    double NL = minSpeed[laneID]/maxSpeed;
    double NA = avgSpeed/maxSpeed;
    double NF = (freeSpace - vehicleCounter[laneID]) / freeSpace;

    return w[0]*NL+ w[1]*NA + w[2]*NF;

}
void TraCIDemo11p::onBeacon(WaveShortMessage* wsm) {
    switch(EVStrategyUsed){
        case EV_FIXED_LANE:
            handleBeaconFixedLane(wsm);
            break;
        case EV_BEST_LANE:
            handleBeaconBestLane(wsm);
            break;
    }
}

void TraCIDemo11p::handleBeaconFixedLane(WaveShortMessage* wsm)
{
    bool isSenderSameLane   = wsm->getLane() == lane;
    bool isSenderAhead      = mobility->getPositionAt(simTime()).x < wsm->getSenderPos().x;
    bool isSenderEV         = wsm->getIsSenderEV();
    double distance2Sender  = abs(wsm->getSenderPos().x - mobility->getPositionAt(simTime()).x);

    if(isSenderEV && isSenderSameLane && !isSenderAhead && distance2Sender < bpd){
        scheduleChangeLane();
    }else if(isSenderEV && !isSenderSameLane && isSenderAhead){
        if(changed) scheduleChangeLane();
    }
}

void TraCIDemo11p::handleBeaconBestLane(WaveShortMessage* wsm)
{
    WaveShortMessage* wsmNew = prepareWSM("data", beaconLengthBits, type_CCH, beaconPriority, wsm->getSenderAddress(), -1);
    sendWSM(wsmNew);
}

void TraCIDemo11p::onDontChange(WaveShortMessage* wsm) {
    EV << "Dont change received \n";
    if(myId == wsm->getRecipientAddress()) dontChange = true;
}

void TraCIDemo11p::onWantChange(WaveShortMessage* wsm) {

    int senderID        = wsm->getSenderAddress();
    double senderSpeed  = wsm->getSpeed();
    double selfSpeed    = traciVehicle->getSpeed();
    double RPURegion    = selfSpeed * 2;
    double FPURegion    = senderSpeed * 2;
    bool isSenderSameLane   = mobility->getPositionAt(simTime()).y != wsm->getSenderPos().y;
    bool isSenderAhead      = mobility->getPositionAt(simTime()).x < wsm->getSenderPos().x;
    double distance2Sender  = abs(wsm->getSenderPos().x - mobility->getPositionAt(simTime()).x);
//    speed_margine = traciVehicle->getSpeed() - pow(traciVehicle->getSpeed()*traciVehicle->getSpeed() - 2*2.9*5.4 , 0.5);

    if(isSenderSameLane){
        if(distance2Sender < URegion) {
            sendDontChange(senderID);
            traciVehicle->slowDown(traciVehicle->getSpeed() * 0.7, 100);
//            traciVehicle->slowDown(senderSpeed,50);
        }
        else if(isSenderAhead){
            if(distance2Sender < RPURegion && senderSpeed < selfSpeed){
//                sendDontChange(senderID);
                traciVehicle->slowDown(wsm->getSpeed()*0.9, 100);
//                traciVehicle->slowDown(senderSpeed,50);
            }
        }else{
            if(distance2Sender < FPURegion && senderSpeed > selfSpeed) {
                sendDontChange(senderID);
                traciVehicle->slowDown(wsm->getSpeed()*0.9, 1000);
            }

//                traciVehicle->slowDown(senderSpeed,50);
        }
    }
}

void TraCIDemo11p::onData(WaveShortMessage* wsm) {
    switch(EVStrategyUsed){
        case EV_FIXED_LANE:
            handleDataFixedLane(wsm);
            break;
        case EV_BEST_LANE:
            handleDataBestLane(wsm);
            break;
    }
}
void TraCIDemo11p::handleDataFixedLane(WaveShortMessage* wsm) {}

void TraCIDemo11p::handleDataBestLane(WaveShortMessage* wsm) {
    if(isEV){
        // check for existance of neighbor
        int senderLane           = wsm->getLane();
        vehicleCounter[senderLane]  += 1;
        cumSpeed[senderLane]        += wsm->getSpeed();

        if (minSpeed[senderLane] > wsm->getSpeed()) minSpeed[senderLane] = wsm->getSpeed();
    }

}

void TraCIDemo11p::sendDontChange(int recID){
//    std::cout<<"Node["<<myId<<"] : sending don't change to = "<< recID << endl;

    WaveShortMessage* wsm = prepareWSM("dont_change", beaconLengthBits, type_CCH, beaconPriority, recID, -1);
    sendWSM(wsm);
//    is_self_affected_by_ev = 1.0;

}

void TraCIDemo11p::sendWSM(WaveShortMessage* wsm) {
    wsm->setSenderPos(mobility->getPositionAt(simTime()));
    wsm->setSpeed(traciVehicle->getSpeed());
    wsm->setSenderAffectedByEV(isAffectedByEV);
    wsm->setIsSenderEV(isEV);
    wsm->setLane(lane);
    sendDelayedDown(wsm,individualOffset);
}

void TraCIDemo11p::scheduleChangeLane(){
    if(!changeLane->isScheduled()){
//        std::cout<<"Node["<<myId<<"] : change scheduled" << endl;
        dontChange = false;
        WaveShortMessage* wsm = prepareWSM("want_change", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
        isAffectedByEV = 1.0;
        sendWSM(wsm);
        scheduleAt(simTime() + 2, changeLane);
    }
}

void TraCIDemo11p::clearUtilMemory()
{
    utilFactor[0]   = 0;//          = {0.0,0.0};utilFactor[0] = 0;
    utilFactor[1]   = 0;
    minSpeed[0]     = traciLane->getMaxSpeed();
    minSpeed[1]     = traciLane->getMaxSpeed();
    cumSpeed[0]     = 0;
    cumSpeed[1]     = 0;
    vehicleCounter[0] = 0;
    vehicleCounter[1] = 0;

    // pop neighbor list

}

bool TraCIDemo11p::neighbourInList(int nID)
{
    for(auto i : neighbors){
        if(i == nID) return true;
    }
    return false;
}

void TraCIDemo11p::finish(){
    recordScalar("is EV",isEV);
    recordScalar("isAffectedByEV", isAffectedByEV);
    switch(EVStrategyUsed){
        case EV_FIXED_LANE:
            recordScalar("bpd", bpd);
            break;
        case EV_BEST_LANE:
            break;
    }


}
