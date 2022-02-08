//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.wireless.handover;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network HandoverNetwork
{
    
    parameters:
        @display("bgi=background/itu_map;bgb=1116,648");
    submodules:
        host: WirelessHost {
            @display("p=51,280;i=device/uav;is=vs");
            @signal[changedTransmitter](type="double");
        }
        ap1: AccessPoint {
            @display("p=109,230;r=,,#707070");
        }
        ap2: AccessPoint {
            @display("p=118,286;r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=60,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=140,50");
        }

        ap3: AccessPoint {
            @display("p=140,394;r=,,#707070");
        }
        ap4: AccessPoint {
            @display("p=148,439;r=,,#707070");
        }
        ap5: AccessPoint {
            @display("p=492,394;r=,,#707070");
        }
        ap6: AccessPoint {
            @display("p=482,370;r=,,#707070");
        }
        ap7: AccessPoint {
            @display("p=475,340;r=,,#707070");
        }
        ap8: AccessPoint {
            @display("p=469,310;r=,,#707070");
        }
        ap9: AccessPoint {
            @display("p=482,280;r=,,#707070");
        }
        ap10: AccessPoint {
            @display("p=531,266;r=,,#707070");
        }
        ap11: AccessPoint {
            @display("p=582,266;r=,,#707070");
        }
        ap12: AccessPoint {
            @display("p=582,316;r=,,#707070");
        }
        ap13: AccessPoint {
            @display("p=588,353;r=,,#707070");
        }
        ap14: AccessPoint {
            @display("p=549,394;r=,,#707070");
        }
        ap15: AccessPoint {
            @display("p=492,155;r=,,#707070");
        }
        ap16: AccessPoint {
            @display("p=549,155;r=,,#707070");
        }
        ap17: AccessPoint {
            @display("p=570,206;r=,,#707070");
        }
        ap18: AccessPoint {
            @display("p=660,244;r=,,#707070");
        }
        ap19: AccessPoint {
            @display("p=660,244;r=,,#707070");
        }
        ap20: AccessPoint {
            @display("p=660,244;r=,,#707070");
        }
       
}

