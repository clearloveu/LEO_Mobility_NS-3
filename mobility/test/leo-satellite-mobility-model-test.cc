#include <ns3/test.h>
#include <ns3/log.h>
#include <ns3/leo-satellite-mobility-model.h>
/**
 * This test verifies the accuracy of the conversion from Polar coordinates to
 * spherical coordinates
 Command to Run this test: ./test.py -v --suite=polar-to-spherical --text=result.txt
**/

NS_LOG_COMPONENT_DEFINE ("LEOSatellitePolarToSphericalTest");

using namespace ns3;

// Some ammount tolerance for testing
const double TOLERANCE = 0.0001;

const double altitude = 100;
const double longitude = 54.8;
const double alpha = 60;
const double inclination = 45; 
const int plane = 2;

const double expected_r = 6478; // altitude + EARTH_RADIUS = (100 + 6378)
const double expected_theta = 1.0472; // DEG_TO_RAD(Alpha) = DEG_TO_RAD(60) = 1.0472    角度到弧度的转换
const double expected_phi = 0.9564; // DEG_TO_RAD(Lon) = DEG_TO_RAD(54.8) = 0.954


class LEOSatellitePolarToSPhericalTestCase: public TestCase
{
public:
  LEOSatellitePolarToSPhericalTestCase (double altitude,
                                        double longitude,
                                        double alpha,
                                        double inclination,
					double plane,
                                        int i);

  virtual ~LEOSatellitePolarToSPhericalTestCase();

private:
  virtual void DoRun (void);
  static std::string Name (double altitude,
                           double longitude,
			   double alpha,
                           double inclination,
                           double plane);
  double m_altitude;
  double m_longitude;
  double m_alpha;
  double m_inclination;
  double m_plane;
  int m_i;
};

std::string
LEOSatellitePolarToSPhericalTestCase::Name (double altitude,
                                            double longitude,
                                            double alpha,
                                            double inclination,
                                            double plane)
{
  std::ostringstream oss;
  oss << "altitude = "    << altitude << " degrees, "
      << "longitude = "   << longitude << " degrees, "
      << "alpha = "       << alpha << " meters, "
      << "inclination = " << inclination << " degrees, "
      << "plane = "       << plane << " number, ";
  return oss.str();
}

LEOSatellitePolarToSPhericalTestCase::LEOSatellitePolarToSPhericalTestCase (double altitude,
                                            double longitude,
                                            double alpha,
                                            double inclination,
                                            double plane,
					    int i)
    : TestCase (Name (altitude, longitude, alpha, inclination, plane)),
    m_altitude (altitude),
    m_longitude (longitude),
    m_alpha (alpha),
    m_inclination (inclination),
    m_plane (plane),
    m_i (i)
{
}

LEOSatellitePolarToSPhericalTestCase::~LEOSatellitePolarToSPhericalTestCase()
{
}

void
LEOSatellitePolarToSPhericalTestCase::DoRun (void)
{
   struct LEOSatPolarPos pPos;
   pPos.altitude = m_altitude;
   pPos.longitude =  m_longitude;
   pPos.alpha =  m_alpha;
   pPos.inclination =  m_inclination;
   pPos.plane =  m_plane;

   LEOSatelliteMobilityModel LEOSat;
   struct LEOSatSphericalPos spPos = LEOSat.m_helper.convertPolarToSpherical(pPos);

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


class LEOSatellitePolarToSPhericalTestSuite : public TestSuite
{
public:
  LEOSatellitePolarToSPhericalTestSuite ();
};


LEOSatellitePolarToSPhericalTestSuite::LEOSatellitePolarToSPhericalTestSuite()
  : TestSuite ("polar-to-spherical", UNIT)
{
  NS_LOG_INFO ("creating PolarToSPhericalTestSuite");
  int i = 0; // iteration number
  AddTestCase (new LEOSatellitePolarToSPhericalTestCase (altitude,
                                                         longitude,
                                                         alpha,
                                                         inclination,
                                                         plane,
                                                         i), TestCase::QUICK);
}

static LEOSatellitePolarToSPhericalTestSuite g_LEOSatellitePolarToSPhericalTestSuite;

