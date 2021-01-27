/**
 * This test verifies the accuracy of the position of terminal after a predefined time interval
 * Command to Run this test: ./test.py -v --suite=leo-satellite-mobility-trace --text=result.txt
**/
#include <ns3/test.h>
#include <ns3/log.h>
#include "ns3/simulator.h"
#include <ns3/leo-satellite-mobility-model.h>
#include <math.h>
#include <fstream>
using namespace std;
NS_LOG_COMPONENT_DEFINE ("VerifyCurrentLEOSatellitePosition");

using namespace ns3;
class LEOSatelliteMobilityTestCase: public TestCase
{
public:
  LEOSatelliteMobilityTestCase();
  virtual ~LEOSatelliteMobilityTestCase();

private:
  void TestPosition (LEOSatSphericalPos expectedPos);
  virtual void DoRun (void);
  Ptr<LEOSatelliteMobilityModel> m_mob;
};


LEOSatelliteMobilityTestCase::LEOSatelliteMobilityTestCase()
 : TestCase ("Test behavior for position after 5 seconds")
{
}

LEOSatelliteMobilityTestCase::~LEOSatelliteMobilityTestCase()
{
}

void LEOSatelliteMobilityTestCase::TestPosition(LEOSatSphericalPos expectedPos)
{
   LEOSatSphericalPos pos= m_mob->DoGetSatSphericalPos();
   NS_TEST_ASSERT_MSG_EQ_TOL (pos.r,
                              expectedPos.r,
                              0.001,
                              "r for (" << pos.r << ") is incorrect "
                              );

   NS_TEST_ASSERT_MSG_EQ_TOL (pos.theta,
                              expectedPos.theta,
                              0.001,
                              "theta for (" << pos.theta << ") is incorrect "
                              );

   NS_TEST_ASSERT_MSG_EQ_TOL (pos.phi,
                              expectedPos.phi,
                              0.001,
                              "phi for (" << pos.phi << ") is incorrect "
                              );
   NS_TEST_ASSERT_MSG_EQ_TOL (pos.inclination,
                              expectedPos.inclination,
                              0.001,
                              "inclination for (" << pos.inclination << ") is incorrect "
                              );

   double x1,y1, z1;
   x1=pos.r*cos(pos.theta)*sin(pos.phi);
   y1=pos.r*sin(pos.theta)*sin(pos.phi);
   z1=pos.r*cos(pos.phi);
   ofstream myfile;
   myfile.open("result_leo.txt", std::ios_base::app);
   myfile << "Expected_r\tActual_r\tExpected_theta\tActual_theta\tExpected_phi\tActual_phi\tEinclination\tAinclination\tEperiod\tAperiod\tX\t\t Y\t\tZ\n"<<expectedPos.r<<"\t\t"<<pos.r<<"\t\t"<<expectedPos.theta<<"\t"<<pos.theta<<"\t"<<expectedPos.phi<<"\t\t"<<pos.phi<<"\t\t"<<expectedPos.inclination<<"\t\t"<<pos.inclination<<"\t\t"<<expectedPos.period<<"\t\t"<<pos.period<<"\t\t"<<x1<<"\t\t"<<y1<<"\t\t"<<z1<<"\n";

}

void LEOSatelliteMobilityTestCase::DoRun(void)
{
  struct LEOSatPolarPos initialpos;
  struct LEOSatSphericalPos expectedPos;
  initialpos.altitude=780;
  initialpos.longitude=8.26;
  initialpos.alpha=118.48;
  initialpos.inclination=86.4;
  initialpos.plane=1;

  expectedPos.r=6378+780; // altitude + EARTH_RADIUS
  expectedPos.theta=2.0678;
  expectedPos.phi=0.14416;
  expectedPos.inclination=1.507; 

  m_mob = CreateObject<LEOSatelliteMobilityModel> ();
  m_mob->setSatSphericalPos(initialpos);
  Simulator::Schedule (Seconds (5.0), &LEOSatelliteMobilityTestCase::TestPosition, this, expectedPos);
  Simulator::Run ();
  //Simulator::Destroy ();
}

class LEOSatelliteMobilityTestSuite : public TestSuite
{
public:
  LEOSatelliteMobilityTestSuite ();
};


LEOSatelliteMobilityTestSuite::LEOSatelliteMobilityTestSuite()
  : TestSuite ("leo-mobility", UNIT)
{
  AddTestCase (new LEOSatelliteMobilityTestCase, TestCase::QUICK);
}

static LEOSatelliteMobilityTestSuite g_LEOSatelliteMobilityTestSuite;

int
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("Scratch Simulator");

  Simulator::Run ();
  Simulator::Destroy ();
}
