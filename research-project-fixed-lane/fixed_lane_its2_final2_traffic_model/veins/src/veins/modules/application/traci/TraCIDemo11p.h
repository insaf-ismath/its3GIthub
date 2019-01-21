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

#ifndef TraCIDemo11p_H
#define TraCIDemo11p_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"



using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

/**
 * Small IVC Demo using 11p
 */
class TraCIDemo11p : public BaseWaveApplLayer {
public:
    enum WaveApplMessageKinds {
        START_SLOW_DOWN_MESSAGE
    };

    enum EVStrategy {
        EV_FIXED_LANE,
        EV_BEST_LANE
    };
	protected:
		std::fstream fs;
        TraCICommandInterface* traci;
		TraCIMobility* mobility;
		TraCICommandInterface::Vehicle* traciVehicle;
		TraCICommandInterface::Lane* traciLane;
		AnnotationManager* annotations;

		cMessage* changeLane;

		static const simsignalwrap_t parkingStateChangedSignal;

		int EVStrategyUsed  = EV_FIXED_LANE;
        bool isEV;
        int lane;
        double bpd;
        double URegion      = (4.3 + 6.5)/2; //Unsafe region = (length of a normal vehicle + length of an ambulance)/2
        double speedLimit   = 22.2;  // maximum speed of a normal vehicle
        double vipSpeed     = 22.2;    // maximum speed of VIP

        // Highway
//      double max_speed_lane[2] = {27.7, 27.7};
        // Suburban
//        double max_speed_lane[2] = {22.2, 22.2};
//         Urban
        double max_speed_lane[2] = {11.11, 11.11};





        double beaconningInterval = 0.909; //Beaconing frequency of VIP and other vehicles

        bool changed    = false;
        bool dontChange = false;

        bool isBVSffectedByEV = 0.0;
        bool isAffectedByEV     = 0;

//        <vehicle arrivalLane="0" color="red" depart="300" departLane="0" id="vip" route="route0" type="CarVIP" />
	public:
        virtual void initialize(int stage);
        virtual void finish();

	protected:
		virtual void handleSelfMsg(cMessage *msg);
        virtual void handleSelfMsgFixedLane(cMessage *msg);
        virtual void handleSelfMsgBestLane(cMessage *msg);

        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void handleBeaconFixedLane(WaveShortMessage* wsm);
        virtual void handleBeaconBestLane(WaveShortMessage* wsm);



		virtual void onData(WaveShortMessage* wsm);
		virtual void onWantChange(WaveShortMessage* wsm);
        virtual void onDontChange(WaveShortMessage* wsm);
		virtual void sendWSM(WaveShortMessage* wsm);
        virtual void sendDontChange(int recID);


        virtual void scheduleChangeLane();
};

#endif
