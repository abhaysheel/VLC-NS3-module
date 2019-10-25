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
 * Author:
 *  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */

#ifndef VLC_CSMACA_H
#define VLC_CSMACA_H

#include <ns3/object.h>
#include <ns3/event-id.h>
#include <ns3/vlc-mac.h>

namespace ns3 {

class UniformRandomVariable;

typedef Callback<void, VlcMacState> VlcMacStateCallback;

class VlcCsmaCa : public Object
{
public:
  static TypeId GetTypeId (void);

  VlcCsmaCa (void);
  virtual ~VlcCsmaCa (void);

  void SetMac (Ptr<VlcMac> mac);

  Ptr<VlcMac> GetMac (void) const;

  void SetSlottedCsmaCa (void);

  void SetUnSlottedCsmaCa (void);

  bool IsSlottedCsmaCa (void) const;

  bool IsUnSlottedCsmaCa (void) const;

  void SetMacMinBE (uint8_t macMinBE);

  uint8_t GetMacMinBE (void) const;

  void SetMacMaxBE (uint8_t macMaxBE);

  uint8_t GetMacMaxBE (void) const;

  void SetMacMaxCSMABackoffs (uint8_t macMaxCSMABackoffs);

  uint8_t GetMacMaxCSMABackoffs (void) const;

  void SetUnitBackoffPeriod (uint64_t unitBackoffPeriod);

  uint64_t GetUnitBackoffPeriod (void) const;

  Time GetTimeToNextSlot (void) const;

  void Start (void);

  void Cancel (void);

  void RandomBackoffDelay (void);

  void CanProceed (void);

  void RequestCCA (void);

  void PlmeCcaConfirm (VlcPhyEnumeration status);

  void SetVlcMacStateCallback (VlcMacStateCallback macState);

  int64_t AssignStreams (int64_t stream);

  uint8_t GetNB (void);

private:

  VlcCsmaCa (VlcCsmaCa const &);

  VlcCsmaCa& operator= (VlcCsmaCa const &);

  virtual void DoDispose (void);

  VlcMacStateCallback m_vlcMacStateCallback;

  bool m_isSlotted;

  Ptr<VlcMac> m_mac;

  uint8_t m_NB;

  uint8_t m_CW;

  uint8_t m_BE;

  bool m_BLE;

  uint8_t m_macMinBE;                   //

  uint8_t m_macMaxBE;

  uint8_t m_macMaxCSMABackoffs;

  uint64_t m_aUnitBackoffPeriod;

  Ptr<UniformRandomVariable> m_random;

  EventId m_randomBackoffEvent;

  EventId m_requestCcaEvent;

  EventId m_canProceedEvent;

  bool m_ccaRequestRunning;
};

}  // namespace ns-3

#endif /* VLC_CSMACA_H */
