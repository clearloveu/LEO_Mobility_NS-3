/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/olsr-helper.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes12;
  nodes12.Create (2);

  NodeContainer nodes23;
  nodes23.Add (nodes12.Get (1));
  nodes23.Create (1);

  NodeContainer nodes34;
  nodes34.Add (nodes23.Get (1));
  nodes34.Create (1);

  NodeContainer nodes41;
  nodes41.Add (nodes12.Get (0));
  nodes41.Add (nodes34.Get (1));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NodeContainer allNodes = NodeContainer (nodes12, nodes23.Get (1), nodes34.Get (1));

  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (allNodes);

  NetDeviceContainer devices12;
  NetDeviceContainer devices23;
  NetDeviceContainer devices34;
  NetDeviceContainer devices41;
  devices12 = pointToPoint.Install (nodes12);
  devices23 = pointToPoint.Install (nodes23);
  devices34 = pointToPoint.Install (nodes34);
  devices41 = pointToPoint.Install (nodes41);

  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper address2;
  address2.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4AddressHelper address3;
  address3.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4AddressHelper address4;
  address4.SetBase ("10.1.4.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces12 = address1.Assign (devices12);
  Ipv4InterfaceContainer interfaces23 = address2.Assign (devices23);
  Ipv4InterfaceContainer interfaces34 = address3.Assign (devices34);
  Ipv4InterfaceContainer interfaces41 = address4.Assign (devices41);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes34.Get (0));
  serverApps.Start (Seconds (31.0));
  serverApps.Stop (Seconds (65.0));

  UdpEchoClientHelper echoClient (interfaces34.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (3.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes12.Get (0));
  clientApps.Start (Seconds (32.0));
  clientApps.Stop (Seconds (65.0));

    Ptr<Node> n1 = nodes12.Get (0);
    Ptr<Ipv4> ipv4_1 = n1->GetObject<Ipv4> ();
    Ptr<Node> n2 = nodes23.Get (0);
    Ptr<Ipv4> ipv4_2 = n2->GetObject<Ipv4> ();

	Ptr<Node> n4 = nodes34.Get (1);
	Ptr<Ipv4> ipv4_4 = n4->GetObject<Ipv4> ();

//    Simulator::Schedule (Seconds (35),&Ipv4::SetDown,ipv4_1, 1);
//    Simulator::Schedule (Seconds (35),&Ipv4::SetDown,ipv4_2, 1);

    Simulator::Schedule (Seconds (33),&Ipv4::SetDown,ipv4_1, 2);
    Simulator::Schedule (Seconds (33),&Ipv4::SetDown,ipv4_4, 2);

    Simulator::Schedule (Seconds (49),&Ipv4::SetUp,ipv4_1, 2);
    Simulator::Schedule (Seconds (49),&Ipv4::SetUp,ipv4_4, 2);

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("first-fourNode-OLSR.tr"));

    //At time 32s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 32.0074s server received 1024 bytes from 10.1.4.1 port 49153
//    At time 32.0074s server sent 1024 bytes to 10.1.4.1 port 49153
//    At time 32.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 35s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 38s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 41s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 41.0074s server received 1024 bytes from 10.1.1.1 port 49153
//    At time 41.0074s server sent 1024 bytes to 10.1.1.1 port 49153
//    At time 41.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 44s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 44.0074s server received 1024 bytes from 10.1.1.1 port 49153
//    At time 44.0074s server sent 1024 bytes to 10.1.1.1 port 49153
//    At time 44.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 47s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 47.0074s server received 1024 bytes from 10.1.1.1 port 49153
//    At time 47.0074s server sent 1024 bytes to 10.1.1.1 port 49153
//    At time 47.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 50s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 50.0074s server received 1024 bytes from 10.1.1.1 port 49153
//    At time 50.0074s server sent 1024 bytes to 10.1.1.1 port 49153
//    At time 50.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 53s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 53.0074s server received 1024 bytes from 10.1.4.1 port 49153
//    At time 53.0074s server sent 1024 bytes to 10.1.4.1 port 49153
//    At time 53.0147s client received 1024 bytes from 10.1.2.2 port 9
//    At time 56s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 56.0074s server received 1024 bytes from 10.1.4.1 port 49153
//    At time 56.0074s server sent 1024 bytes to 10.1.4.1 port 49153
//    At time 56.0147s client received 1024 bytes from 10.1.3.1 port 9
//    At time 59s client sent 1024 bytes to 10.1.3.1 port 9
//    At time 59.0074s server received 1024 bytes from 10.1.4.1 port 49153
//    At time 59.0074s server sent 1024 bytes to 10.1.4.1 port 49153
//    At time 59.0147s client received 1024 bytes from 10.1.3.1 port 9



  Simulator::Stop (Seconds (66.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
