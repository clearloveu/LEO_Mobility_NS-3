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
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-nix-vector-routing.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NixSimpleExample");


std::vector< Ptr<Node> >
buildParentVector(std::vector< Ptr<Node> > & path){
	std::vector< Ptr<Node> > parentVector;
	// reset the parent vector
	parentVector.clear ();
	uint32_t numberOfNodes = NodeList::GetNNodes ();
    parentVector.reserve (sizeof (Ptr<Node>)*numberOfNodes);
    parentVector.insert (parentVector.begin (), sizeof (Ptr<Node>)*numberOfNodes, 0); // initialize to 0

	int size = path.size();
	if(size <= 0 ) return parentVector;
	Ptr<Node> pre = path.at(0);
	parentVector.at (pre->GetId ()) = pre;
	Ptr<Node> current;
	for(int i = 1; i < size; i++) {
		current = path.at(i);
		parentVector.at (current->GetId ()) = pre;
		pre = current;
	}
	return parentVector;
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("Ipv4NixVectorRouting", LOG_LEVEL_LOGIC);

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

  // NixHelper to install nix-vector routing
  // on all nodes
  Ipv4NixVectorHelper nixRouting;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (nixRouting, 10);

  InternetStackHelper stack;
  stack.SetRoutingHelper (list); // has effect on the next Install ()
  stack.Install (allNodes);

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
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces34.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (5.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes12.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // Trace routing tables
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("nix-simple.routes", std::ios::out);
  nixRouting.PrintRoutingTableAllAt (Seconds (8), routingStream);

  // 构造UpdateNixVectorInCache 参数，做实验
  // (Ipv4Address destinationIp, const std::vector< Ptr<Node> > & parentVector, Ptr<Node> source, Ptr<Node> dest)
  Ipv4Address destinationIp = interfaces34.GetAddress (0);
  Ptr<Node> source = nodes12.Get(0);
  Ptr<Node> dest = nodes34.Get(0);
  std::vector< Ptr<Node> > path = {nodes12.Get(0), nodes34.Get(1), nodes34.Get(0)};
  std::vector< Ptr<Node> > parentVector = buildParentVector(path);

  Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
//  Simulator::Schedule (Seconds (4),&Ipv4NixVectorRouting::SetCacheDirty, rp ,true);
  Simulator::Schedule (Seconds (4),&Ipv4NixVectorRouting::UpdateNixVectorInCache, rp ,destinationIp, parentVector, source, dest); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
