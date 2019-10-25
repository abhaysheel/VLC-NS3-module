/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 *
 * Author: Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#ifndef VLC_ERROR_MODEL_H
#define VLC_ERROR_MODEL_H

#include <ns3/object.h>

namespace ns3 {

/**
 * \ingroup vlc
 *
 * Model the error rate for IEEE 802.15.7 2.4 GHz AWGN channel for phyII-OOK
 */
class VlcErrorModel : public Object
{
public:

  static TypeId GetTypeId (void);

  VlcErrorModel (void);

  double GetChunkSuccessRate (double snr, uint32_t nbits) const;

private:

  double m_binomialCoefficients[17];

};


} // namespace ns3

#endif /* VLC_ERROR_MODEL_H */
