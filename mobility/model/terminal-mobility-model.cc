#include "terminal-mobility-model.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TerminalMobilityModel);

TypeId TerminalMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TerminalMobilityModel")
      .SetParent<MobilityModel> ()
      .SetGroupName ("Mobility")
      .AddConstructor<TerminalMobilityModel> ();
      return tid;
}

TerminalMobilityModel::TerminalMobilityModel ()
{
}
 
TerminalMobilityModel::~TerminalMobilityModel ()
{
}

TerminalSphericalPos
TerminalMobilityModel::DoGetTermSphericalPos(void) const
{
   m_helper.Update();
   return m_helper.GetCurrentPos();
}

void
TerminalMobilityModel::DoSetTermSphericalPos(const TerminalPolarPos& pos)
{
  m_helper.SetPos(pos);
  m_helper.Unpause();
  NotifyCourseChange();
}

void
TerminalMobilityModel::setSphericalPos(const TerminalPolarPos& polarPos)
{
  DoSetTermSphericalPos(polarPos);
}

Vector
TerminalMobilityModel::DoGetPosition (void) const
{
  m_helper.Update ();
  return Vector(0.0, 0.0, 0.0); //m_helper.GetCurrentPos();
}
void
TerminalMobilityModel::DoSetPosition (const Vector &position)
{
  //m_helper.SetPos(position);
  //NotifyCourseChange();
}

Vector
TerminalMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}


} // namespace ns3
