#include "ns3/simulator.h"
#include "ns3/log.h"
#include "terminal-mobility-model.h"

#define PI 3.1415926535897
#define MU 398601.2 // Greek Mu (km^3/s^2)
#define LIGHT 299793 // km/s
#define EARTH_PERIOD 86164 // seconds
#define EARTH_RADIUS 6378  // km
#define GEO_ALTITUDE 35786 // km
#define ATMOS_MARGIN 150 // km

#define DEG_TO_RAD(x) ((x) * PI/180)
#define RAD_TO_DEG(x) ((x) * 180/PI)
#define DISTANCE(s_x, s_y, s_z, e_x, e_y, e_z) (sqrt((s_x - e_x) * (s_x - e_x) \
                + (s_y - e_y) * (s_y - e_y) + (s_z - e_z) * (s_z - e_z)))
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TerminalHelper");

TerminalSphericalPos
TerminalHelper::computeCurPos(double timeAdvance) const
{
  TerminalSphericalPos initialSphericalPos =  m_pos;
  TerminalSphericalPos curSphericalPos;
  curSphericalPos.r = initialSphericalPos.r;
  curSphericalPos.theta = initialSphericalPos.theta;
  curSphericalPos.phi = fmod((initialSphericalPos.phi + 
	    (fmod(timeAdvance,initialSphericalPos.period)/initialSphericalPos.period) * 2*PI), 2*PI);
  return curSphericalPos;
}

TerminalSphericalPos
TerminalHelper::convertPolarToSpherical(const TerminalPolarPos &polarPos)
{
   NS_LOG_INFO(this << polarPos.latitude << polarPos.longitude);
   TerminalSphericalPos sphericalPos;
   // Check validity of passed values.
  
   if ((polarPos.longitude < -180) || (polarPos.longitude) > 180)
   {
      NS_LOG_FUNCTION(this << "Longitude out of bounds");
   }
   if ((polarPos.latitude < -90) || (polarPos.latitude) > 90)
   {
      NS_LOG_FUNCTION(this << "Latitude out of bounds");
   }

   m_paused = true;
   sphericalPos.r = EARTH_RADIUS;
   sphericalPos.theta = DEG_TO_RAD(90-polarPos.latitude);
   if (polarPos.longitude < 0)
   {
     sphericalPos.phi = DEG_TO_RAD(360 + polarPos.longitude);
   }
   else
   {
      sphericalPos.phi = DEG_TO_RAD(polarPos.longitude);
   }
  
   //double num = sphericalPos.r * sphericalPos.r * sphericalPos.r;
   sphericalPos.period = EARTH_PERIOD;
   return sphericalPos;
}

TerminalHelper::TerminalHelper()
     : m_paused (true)
{
   NS_LOG_INFO(this);
}


TerminalHelper::TerminalHelper (const TerminalPolarPos &polarPos)
{
   NS_LOG_FUNCTION(this);
   NS_LOG_INFO (this << polarPos.latitude << polarPos.longitude);

   // Check validity of passed values.
  
   if ((polarPos.longitude < -180) || (polarPos.longitude) > 180) 
   {
      NS_LOG_FUNCTION(this << "Longitude out of bounds");
   }
   if ((polarPos.latitude < -90) || (polarPos.latitude) > 90) 
   {
      NS_LOG_FUNCTION(this << "Latitude out of bounds");
   }

  m_paused = true;
  m_pos.r = EARTH_RADIUS;
  m_pos.theta = DEG_TO_RAD(90-polarPos.latitude);
  if (polarPos.longitude < 0)
  {
     m_pos.phi = DEG_TO_RAD(360 + polarPos.longitude);
  }
  else
  {
      m_pos.phi = DEG_TO_RAD(polarPos.longitude);
  }
  
  //double num = m_pos.r * m_pos.r * m_pos.r;
  m_pos.period = EARTH_PERIOD;
}


void
TerminalHelper::SetPos(const TerminalPolarPos& pos)
{
   NS_LOG_INFO(this << "latitude " << pos.latitude << "longitude " << pos.longitude);
   m_pos = convertPolarToSpherical(pos);
   m_lastUpdate = Simulator::Now();
}

TerminalSphericalPos
TerminalHelper::GetCurrentPos(void) const
{
  NS_LOG_FUNCTION (this);
  return m_pos;
}

Vector
TerminalHelper::GetVelocity (void) const
{
  NS_LOG_FUNCTION (this);
  //return m_paused ? Vector (0.0, 0.0, 0.0) : m_velocity;
  return m_velocity;
}

void
TerminalHelper::Update (void) const
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now();
  NS_ASSERT(m_lastUpdate <= now);
  Time deltaTime = now - m_lastUpdate;
  m_lastUpdate = now;
  if (m_paused)
  {
    NS_LOG_INFO(this << "\npaused");
    return;
  }
  double deltaS = deltaTime.GetSeconds();
  m_pos = computeCurPos(deltaS);
}

void
TerminalHelper::Pause (void)
{
   NS_LOG_FUNCTION (this);
   m_paused = true;
}


void
TerminalHelper::Unpause (void)
{
   NS_LOG_FUNCTION (this);
   m_paused = false;
}
 
} // namespace ns3
