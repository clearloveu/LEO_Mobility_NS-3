#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/terminal-mobility-model.h"
/**
 * This test verifies the accuracy of the conversion from Polar coordinates to
 * spherical coordinates
 Command to Run this test: ./test.py -v --suite=polar-to-spherical1 --text=result.txt
**/

NS_LOG_COMPONENT_DEFINE ("TerminalPolarToSphericalTest");

using namespace ns3;

// Some amount tolerance for testing
const double TOLERANCE = 0.0001;

const double longitude = 54.8;
const double latitude = 54.8;

const double expected_r = 6378; // EARTH_RADIUS = (6378)
const double expected_theta = 0.6144; // DEG_TO_RAD(90-latitude) = DEG_TO_RAD(90-54.8) = 0.6144
const double expected_phi = 0.9564; // DEG_TO_RAD(Lon) = DEG_TO_RAD(54.8) = 0.9564


class TerminalPolarToSPhericalTestCase: public TestCase
{
public:
  TerminalPolarToSPhericalTestCase (double latitude,
                                       double longitude,
                                        int i);

  virtual ~TerminalPolarToSPhericalTestCase();

private:
  virtual void DoRun (void);
  static std::string Name (double latitude,
                           double longitude);
  double m_latitude;
  double m_longitude;
  int m_i;
};

std::string
TerminalPolarToSPhericalTestCase::Name (double latitude,
                                            double longitude)
{
  std::ostringstream oss;
  oss << "latitude = "    << latitude << " degrees, "
      << "longitude = "   << longitude << " degrees, ";
     
  return oss.str();
}
TerminalPolarToSPhericalTestCase::TerminalPolarToSPhericalTestCase (double latitude,
                                            double longitude,
					    int i)
    : TestCase (Name (latitude, longitude)),
    m_latitude (latitude),
    m_longitude (longitude),
    m_i (i)
{
}

TerminalPolarToSPhericalTestCase::~TerminalPolarToSPhericalTestCase()
{
}

void
TerminalPolarToSPhericalTestCase::DoRun (void)
{
   struct TerminalPolarPos pPos;
   pPos.latitude = m_latitude;
   pPos.longitude =  m_longitude;
  

   TerminalMobilityModel Term;
   struct TerminalSphericalPos spPos = Term.m_helper.convertPolarToSpherical(pPos);

   NS_TEST_ASSERT_MSG_EQ_TOL (spPos.r,
                              expected_r,
                              TOLERANCE,
                              "r for (" << spPos.r << ") is incorrect "
                              "in iteration" << m_i);

   NS_TEST_ASSERT_MSG_EQ_TOL (spPos.theta,
                              expected_theta,
                              TOLERANCE,
                              "theta for (" << spPos.theta << ") is incorrect "
                              "in iteration" << m_i);
 
   NS_TEST_ASSERT_MSG_EQ_TOL (spPos.phi,
                              expected_phi,
                              TOLERANCE,
                              "phi for (" << spPos.phi << ") is incorrect "
                              "in iteration" << m_i);
}


class TerminalPolarToSPhericalTestSuite : public TestSuite
{
public:
  TerminalPolarToSPhericalTestSuite ();
};


TerminalPolarToSPhericalTestSuite::TerminalPolarToSPhericalTestSuite()
  : TestSuite ("polar-to-spherical1", UNIT)
{
  NS_LOG_INFO ("creating PolarToSPhericalTestSuite");
  int i = 0; // iteration number
  AddTestCase (new TerminalPolarToSPhericalTestCase (latitude,
                                                         longitude,
                                                         i), TestCase::QUICK);
}

static TerminalPolarToSPhericalTestSuite g_TerminalPolarToSPhericalTestSuite;
