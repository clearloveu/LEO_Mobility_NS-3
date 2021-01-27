#include <ns3/test.h>
#include <ns3/log.h>
#include "ns3/simulator.h"
#include <ns3/terminal-mobility-model.h>
/**
 * This test verifies the accuracy of the position of terminal after a predefined time interval
 * Command to Run this test: ./test.py -v --suite=terminal-mobility --text=result.txt
**/

NS_LOG_COMPONENT_DEFINE ("VerifyCurrentTerminalPosition");

using namespace ns3;
class TerminalMobilityTestCase: public TestCase
{
public:
  TerminalMobilityTestCase();
  virtual ~TerminalMobilityTestCase();

private:
  void TestPosition (TerminalSphericalPos expectedPos);
  virtual void DoRun (void);
  Ptr<TerminalMobilityModel> m_mob;
};


TerminalMobilityTestCase::TerminalMobilityTestCase()
 : TestCase ("Test behavior for position after 5 seconds")
{
}

TerminalMobilityTestCase::~TerminalMobilityTestCase()
{
}
void TerminalMobilityTestCase::TestPosition(TerminalSphericalPos expectedPos)
{
   TerminalSphericalPos pos= m_mob->DoGetTermSphericalPos();
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


}
void TerminalMobilityTestCase::DoRun(void)
{
  struct TerminalPolarPos initialpos;
  struct TerminalSphericalPos expectedPos;
  initialpos.latitude=58.4;
  initialpos.longitude=58.4;
  expectedPos.r=6378;
  expectedPos.theta=0.551524;
  expectedPos.phi=1.019636606;
  expectedPos.period=86164;
  m_mob = CreateObject<TerminalMobilityModel> ();
  m_mob->DoSetTermSphericalPos(initialpos);
  Simulator::Schedule (Seconds (5.0), &TerminalMobilityTestCase::TestPosition, this, expectedPos);
  Simulator::Run ();
  Simulator::Destroy ();
}

class TerminalMobilityTestSuite : public TestSuite
{
public:
  TerminalMobilityTestSuite ();
};


TerminalMobilityTestSuite::TerminalMobilityTestSuite()
  : TestSuite ("terminal-mobility", UNIT)
{
  //NS_LOG_INFO ("creating TerminalMobilityTestSuite");
  //int i = 0; iteration number
  AddTestCase (new TerminalMobilityTestCase, TestCase::QUICK);
}

static TerminalMobilityTestSuite g_TerminalMobilityTestSuite;

