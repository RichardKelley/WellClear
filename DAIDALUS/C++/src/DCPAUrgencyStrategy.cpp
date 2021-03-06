/*
 * DCPAUrgencyStrategy.cpp
 *
 * Most urgent strategy based on distance at closest point of approach. When this distance is less than the minimum
 * recovery separation given by D and H, time to closest point of approach is used.
 *
 */

#include "ACCoRDConfig.h"
#include "CD3D.h"
#include "DCPAUrgencyStrategy.h"

namespace larcfm {

TrafficState DCPAUrgencyStrategy::mostUrgentAircraft(Detection3D* detector, const OwnshipState& ownship, const std::vector<TrafficState>& traffic, double T) {
  TrafficState repac = TrafficState::INVALID;
  if (!ownship.isValid() || traffic.empty()) {
    return repac;
  }
  double mindcpa = 0;
  double mintcpa = 0;
  double D = ACCoRDConfig::NMAC_D;
  double H = ACCoRDConfig::NMAC_H;
  Vect3 so = ownship.get_s();
  Velocity vo = ownship.get_v();
  for (int i = 0; i < traffic.size(); ++i) {
    Vect3 si = ownship.pos_to_s(traffic[i].getPosition());
    Velocity vi = ownship.vel_to_v(traffic[i].getPosition(),traffic[i].getVelocity());
    Vect3 s = so.Sub(si);
    Velocity v = vo.Sub(vi);
    ConflictData det = detector->conflictDetection(so,vo,si,vi,0,T);
    if (det.conflict()) {
      double tcpa = CD3D::tccpa(s,vo,vi,D,H);
      double dcpa = v.ScalAdd(tcpa,s).cyl_norm(D,H);
      // If aircraft have almost same tcpa, select the one with smallest dcpa
      // Otherwise,  select aircraft with smallest tcpa
      bool tcpa_strategy = Util::almost_equals(tcpa,mintcpa,PRECISION5) ? dcpa < mindcpa : tcpa < mintcpa;
      // If aircraft have almost same dcpa, select the one with smallest tcpa
      // Otherwise,  select aircraft with smallest dcpa
      bool dcpa_strategy = Util::almost_equals(dcpa,mindcpa,PRECISION5) ? tcpa < mintcpa : dcpa < mindcpa;
     // If aircraft are both in a min recovery trajectory, follows tcpa strategy. Otherwise follows dcpa strategy
      if (!repac.isValid() || // There are no candidates
          (dcpa <= 1 ? mindcpa > 1 || tcpa_strategy : dcpa_strategy)) {
        repac = traffic[i];
        mindcpa = dcpa;
        mintcpa = tcpa;
      }
    }
  }
  return repac;
}

UrgencyStrategy* DCPAUrgencyStrategy::copy() const {
  return new DCPAUrgencyStrategy();
}

}
