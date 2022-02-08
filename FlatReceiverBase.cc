//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
//#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ReceiverBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/IInterference.h"

#include <iostream>
#include <fstream>
namespace inet {

namespace physicallayer {

FlatReceiverBase::FlatReceiverBase() :
    NarrowbandReceiverBase(),
    errorModel(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN))
{
}

void FlatReceiverBase::initialize(int stage)
{
    NarrowbandReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
    }
}

std::ostream& FlatReceiverBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", errorModel = " << printObjectToString(errorModel, level - 1);
    if (level >= PRINT_LEVEL_INFO)
        stream << ", energyDetection = " << energyDetection
               << ", sensitivity = " << sensitivity;
    return NarrowbandReceiverBase::printToStream(stream, level);
}

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *narrowbandNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = narrowbandNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing whether listening is possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}



void writelogfile(std::string filename, simtime_t time, std::string coord, std::string channel, double rssi, std::string transmitter)
{
         coord.erase(0,1);
         coord.erase(coord.size()-1);
         std::vector<std::string> result;
         std::stringstream s_stream(coord);

         std::string x,y,z; //coordinates
         while(s_stream.good()){
             std::string substr;
             std::getline(s_stream,substr,',');
             result.push_back(substr);
         }

         x = result.at(0);
         y = result.at(1);
         z = result.at(2);

         y.erase(0,1);
         z.erase(0,1);
         std::ofstream  out;
         out.open(filename.c_str(), std::ios_base::app);
         out << time << "," << x << "," << y << "," << z << "," << channel << "," << rssi << "," << transmitter << "\n";
         out.close();
}

void FlatReceiverBase::setApPower(std::string apName,double amount = 0,int choice = 0) const
{
    cModule *ap = getParentModule()->getParentModule()->getParentModule()->getParentModule()->getSubmodule(apName.c_str(),0);

    cModule *ap_wlan_0 = ap->getSubmodule("wlan",0)->getSubmodule("radio")->getSubmodule("transmitter");
    cModule *ap_wlan_1 = ap->getSubmodule("wlan",1)->getSubmodule("radio")->getSubmodule("transmitter");

    FlatTransmitterBase *ap_powered_0 = check_and_cast<FlatTransmitterBase *>(ap_wlan_0);
    FlatTransmitterBase *ap_powered_1 = check_and_cast<FlatTransmitterBase *>(ap_wlan_1);

    switch(choice)
    {
    case 1:
        ap_powered_0->setPower(W(ap_powered_0->getPower()+W(amount)));
        ap_powered_1->setPower(W(ap_powered_1->getPower()+W(amount)));
        break;
    case -1:
        if(ap_powered_0->getPower()>W(0))
        {
            ap_powered_0->setPower(W(ap_powered_0->getPower()-W(amount)));
            ap_powered_1->setPower(W(ap_powered_1->getPower()-W(amount)));
            break;
        }
        else{
            ap_powered_0->setPower(W(0));
            ap_powered_1->setPower(W(0));
            break;
        }
    }

    //std::cout << apName << ": "<< ap_powered_0->getPower() << endl;
}

// TODO: this is not purely functional, see interface comment
bool FlatReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    if (!NarrowbandReceiverBase::computeIsReceptionPossible(listening, reception, part))
        return false;
    else {
        const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
        W minReceptionPower = flatReception->computeMinPower(reception->getStartTime(part), reception->getEndTime(part));
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing whether reception is possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;

/*
        //----------- TRANSMISSION PART -----------//
        const ITransmission* transmission = flatReception->getTransmission();

        const cPacket* transmitter_packet = transmission->getMacFrame();
        cModule* arrival_module = transmitter_packet->getArrivalModule();
        cModule* transmitter_module = arrival_module->getParentModule()->getParentModule();
        cModule* receiver_module = getParentModule()->getParentModule()->getParentModule();
        cObject *lastPosition = getParentModule()->getParentModule()->getParentModule()->getParentModule()->getSubmodule("host",0)->getSubmodule("mobility",0)->findObject("lastPosition",false);
        cObject *channel = receiver_module->getSubmodule("wlan",0)->getSubmodule("radio",0)->findObject("channelNumber",false);

        cModule *channelReceiverModule = receiver_module->getSubmodule("wlan",0)->getSubmodule("radio")->getSubmodule("receiver");
        cModule *channelTransmitterModule = receiver_module->getSubmodule("wlan",0)->getSubmodule("radio")->getSubmodule("transmitter");

        int channelNo = 5;
        Ieee80211ReceiverBase *flatRChannel = check_and_cast<Ieee80211ReceiverBase *>(channelReceiverModule);
        Ieee80211TransmitterBase *flatTChannel = check_and_cast<Ieee80211TransmitterBase *>(channelTransmitterModule);

        flatRChannel->setChannelNumber(channelNo);
        flatTChannel->setChannelNumber(channelNo);


        //setApPower("ap3",0.0001,-1);

        //--------- NOISE -----------//


        //--------- CONSOLE PART ---------//

        double rssi = inet::math::mW2dBm(minReceptionPower.get() * 1000);

        if(std::string(receiver_module->getFullName()) == "host"){
        std::cout << " SimTime: " << simTime() << " sec"
                  << " hostCoord: " << lastPosition->info()
                  << " Channel: " << channel->info()
                  << " RSSI: " << rssi << " dBm "
                  << " (T -> R): "<<"(" <<transmitter_module->getFullName()
                  << " -> "
                  << receiver_module->getFullName() << ")"
                <<endl;
        writelogfile("handover.csv",simTime(),lastPosition->info(),channel->info(),rssi,transmitter_module->getFullName());
        }
*/
        return isReceptionPossible;
    }
}


bool FlatReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const
{
    if (!SNIRReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir))
        return false;
    else if (!errorModel)
        return true;
    else {
        double packetErrorRate = errorModel->computePacketErrorRate(snir, part);
        if (packetErrorRate == 0.0)
            return true;
        else if (packetErrorRate == 1.0)
            return false;
        else
            return dblrand() > packetErrorRate;
    }
}

const ReceptionIndication *FlatReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    ReceptionIndication *indication = const_cast<ReceptionIndication *>(SNIRReceiverBase::computeReceptionIndication(snir));
    indication->setPacketErrorRate(errorModel ? errorModel->computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    indication->setBitErrorRate(errorModel ? errorModel->computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    indication->setSymbolErrorRate(errorModel ? errorModel->computeSymbolErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    return indication;
}

} // namespace physicallayer

} // namespace inet

