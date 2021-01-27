/* Constellation has 6 planes. Each plane contains 11 satellites.
 * Each satellite has a node number, altitude, inclination, longitude, alpha and plane.
 * Altitude and inclination are constant for all satellites in the constellation
 * export 'NS_LOG=IridiumConstellation=level_all|prefix_func|prefix_time'
 * ./waf --run "test-runner --test-name=iridium-constellation"
*/

const double ALTITUDE = 780;
const double INCLINATION = 86.4;
const double TOLERANCE = 10;
// Format: nodenumber, longitude, alpha and plane.
double iridiumConstellation[3][11][4] = {{{0, 0, 0, 0}, {1, 0, 32.73, 0}, {2, 0, 65.45, 0}, {3, 0, 98.18, 0},
                                          {4, 0, 130.91, 0}, {5, 0, 163.64, 0}, {6, 0, 196.36, 0},{7, 0, 229.09, 0},
                                          {8, 0, 261.82, 0}, {9, 0, 294.55, 0}, {10, 0, 327.27, 0}},
                                         {{11, 31.6, 16.36, 1}, {12, 31.6, 49.09, 1}, {13, 31.6, 81.82, 1}, {14, 31.6, 114.55, 1},
                                          {15, 31.6, 147.27, 1}, {16, 31.6, 180, 1}, {17, 31.6, 212.73, 1}, {18, 31.6, 245.45, 1},
                                          {19, 31.6, 278.18, 1}, {20, 0, 310.91, 1}, {21, 0, 343.64, 1}},
                                         {{22, 63.2, 0, 2}, {23, 63.2, 32.73, 2}, {24, 63.2, 65.45, 2}, {24, 63.2, 98.18, 2},
                                          {25, 63.2, 130.91, 2}, {26, 63.2, 163.64, 2}, {27, 63.2, 196.36, 2}, {28, 63.2, 229.09, 2},
                                          {29, 63.2, 261.82, 2}, {30, 63.2, 294.55, 2}, {31, 63.2, 327.27, 2}}};

#include <ns3/test.h>
#include <ns3/log.h>
#include "ns3/simulator.h"
#include <ns3/leo-satellite-mobility-model.h>
#include <math.h>
#include <fstream>

using namespace std;
NS_LOG_COMPONENT_DEFINE ("IridiumConstellation");

using namespace ns3;

class IridiumConstellationTestCase: public TestCase
{
public:
  IridiumConstellationTestCase(double altitude,
                               double longitude,
                               double alpha,
                               double inclination,
                               double plane,
                               int i,
			       int nodeId);
  virtual ~IridiumConstellationTestCase();

private:
  virtual void DoRun (void);
  void FindNewPosition(void);
  static std::string Name (double altitude,
                           double longitude,
                           double alpha,
                           double inclination,
                           double plane,
                           int i,
		           int nodeId);
  double m_altitude;
  double m_longitude;
  double m_alpha;
  double m_inclination;
  double m_plane;
  int m_i;
  int m_nodeId;
  Ptr<LEOSatelliteMobilityModel> m_mob;
};

std::string
IridiumConstellationTestCase::Name (double altitude,
                                    double longitude,
                                    double alpha,
                                    double inclination,
                                    double plane,
                                    int i,
				    int nodeId)
{
  std::ostringstream oss;
  oss << "altitude = "    << altitude << " degrees, "
      << "longitude = "   << longitude << " degrees, "
      << "alpha = "       << alpha << " meters, "
      << "inclination = " << inclination << " degrees, "
      << "plane = "       << plane << " number, ";
  return oss.str();
}

IridiumConstellationTestCase::IridiumConstellationTestCase (double altitude,
                                            double longitude,
                                            double alpha,
                                            double inclination,
                                            double plane,
                                            int i, 
					    int nodeId)
    : TestCase (Name (altitude, longitude, alpha, inclination, plane, i, nodeId)),
    m_altitude (altitude),
    m_longitude (longitude),
    m_alpha (alpha),
    m_inclination (inclination),
    m_plane (plane),
    m_i (i),
    m_nodeId(nodeId)
{
}

IridiumConstellationTestCase::~IridiumConstellationTestCase()
{
}

void
IridiumConstellationTestCase::FindNewPosition(void)
{
  LEOSatSphericalPos spPos = m_mob->DoGetSatSphericalPos();
  double x1,y1, z1;
  x1=spPos.r*sin(spPos.theta)*cos(spPos.phi);
  y1=spPos.r*sin(spPos.theta)*sin(spPos.phi);
  z1=spPos.r*cos(spPos.theta);
  ofstream myfile;
  myfile.open("result_iridium_new_plot.txt", std::ios_base::app);
  //myfile << "[" << m_nodeId<<","<< spPos.r << ",\t" << spPos.theta << ",\t" << spPos.phi << ",\t" <<  x1<<",\t"<<y1<<",\t"<<z1 << "],"<< "\n";
   myfile<<x1<<"\t"<<y1<<"\t"<<z1<<"\n";

  NS_LOG_INFO (this << "After update" << "x" << x1 << "y" << y1 << "z" << z1 << "\n");

}

void
IridiumConstellationTestCase::DoRun (void)
{
   struct LEOSatPolarPos pPos;
   pPos.altitude = m_altitude;
   pPos.longitude =  m_longitude;
   pPos.alpha =  m_alpha;
   pPos.inclination =  m_inclination;
   pPos.plane =  m_plane;
   m_mob = CreateObject<LEOSatelliteMobilityModel> ();
   m_mob->DoSetSatSphericalPos(pPos);

   LEOSatelliteMobilityModel LEOSat;
   struct LEOSatSphericalPos spPos = LEOSat.m_helper.convertPolarToSpherical(pPos);
   NS_LOG_INFO (this << " radius" << spPos.r << " theta" << spPos.theta << " phi" << spPos.phi << "\n");

   double x1,y1, z1;
   x1=spPos.r*sin(spPos.theta)*cos(spPos.phi);
   y1=spPos.r*sin(spPos.theta)*sin(spPos.phi);
   z1=spPos.r*cos(spPos.theta);
   ofstream myfile;
   myfile.open("result_iridium_plot.txt", std::ios_base::app);
   //myfile << "[" << m_nodeId<<","<< spPos.r << ",\t" << spPos.theta << ",\t" << spPos.phi << ",\t" <<  x1<<",\t"<<y1<<",\t"<<z1 << "],"<< "\n";
   //myfile<<x1<<"\t"<<y1<<"\t"<<z1<<"\n";
   NS_LOG_INFO (this << "before update x" << x1 << "y" << y1 << "z" << z1 << "\n");
   m_mob = CreateObject<LEOSatelliteMobilityModel> ();
   m_mob->DoSetSatSphericalPos(pPos);
   Simulator::Schedule (Minutes(10), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(20), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(30), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(40), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(50), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(60), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(70), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(80), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(90), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(100), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(110), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(120), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(130), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(140), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(150), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(160), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(170), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(180), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(190), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(200), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(210), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(220), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(230), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(240), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Schedule (Minutes(250), &IridiumConstellationTestCase::FindNewPosition, this);
   Simulator::Run ();
   Simulator::Destroy ();
}


class IridiumConstellationTestSuite : public TestSuite
{
public:
  IridiumConstellationTestSuite ();
};

IridiumConstellationTestSuite::IridiumConstellationTestSuite()
                              : TestSuite ("iridium-constellation", UNIT)
{
  //int i = 0; // iteration number
  //Generate a node for each iridium satellite.
  /*for(int planes = 0; planes < 1; planes++)
  {
    for(int j = 0; j < 1; j++)
    {
      double *sat={iridiumConstellation[planes][i]};
      double altitude = ALTITUDE;
      double longitude = sat[1];
      double alpha = sat[2];
      double inclination = INCLINATION;
      double plane = sat[3];
      int node_num = sat[0];*/
      AddTestCase (new IridiumConstellationTestCase (780,
						     0,
						     0,
                                                     86.4,
                                                     0,
                                                     0,
                                                     0), TestCase::QUICK);
    // i++;
 
    //} 
  //}
}
static IridiumConstellationTestSuite g_IridiumConstellationTestSuite;
