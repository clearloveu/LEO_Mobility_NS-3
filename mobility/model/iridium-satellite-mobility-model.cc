#include "iridium-satellite-mobility-model.h"
#include "ns3/simulator.h"

NS_OBJECT_ENSURE_REGISTERED (IridiumSatelliteMobilityModel);

TypeId IridiumSatelliteMobilityModel:GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IridiumSatelliteMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<IridiumSatelliteMobilityModel> ();
  return tid;
}

IridiumSatelliteMobilityModel::IridiumSatelliteMobilityModel()
{
}

IridiumSatelliteMobilityModel::~IridiumSatelliteMobilityModel()
{
}

inline Vector
IridiumSatelliteMobilityModel::DoGetVelocity (void) const
{
  double t = (Simulator::Now () - m_baseTime).GetSeconds ();
  return Vector (m_baseVelocity.x + m_acceleration.x*t,
                 m_baseVelocity.y + m_acceleration.y*t,
                 m_baseVelocity.z + m_acceleration.z*t);
}

inline Vector
IridiumSatelliteMobilityModel::DoGetPosition (void) const
{
  double t = (Simulator::Now () - m_baseTime).GetSeconds ();
  double half_t_square = t*t*0.5;
  return Vector (m_basePosition.x + m_baseVelocity.x*t + m_acceleration.x*half_t_square,
                 m_basePosition.y + m_baseVelocity.y*t + m_acceleration.y*half_t_square,
                 m_basePosition.z + m_baseVelocity.z*t + m_acceleration.z*half_t_square);
}

void
IridiumSatelliteMobilityModel::DoSetPosition (const Vector &position)
{
  m_baseVelocity = DoGetVelocity ();
  m_baseTime = Simulator::Now ();
  m_basePosition = position;
  NotifyCourseChange ();
}

void
IridiumSatelliteMobilityModel::SetVelocityAndAcceleration (const Vector &velocity,
                                                           const Vector &acceleration)
{
  m_basePosition = DoGetPosition ();
  m_baseTime = Simulator::Now ();
  m_baseVelocity = velocity;
  m_acceleration = acceleration;
  NotifyCourseChange ();
}


} // namespace ns3
                      


