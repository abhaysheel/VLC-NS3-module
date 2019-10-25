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

#include "vlc-csmaca.h"
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <algorithm>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("VlcCsmaCa");

  NS_OBJECT_ENSURE_REGISTERED (VlcCsmaCa);

  TypeId
  VlcCsmaCa::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::VlcCsmaCa")
      .SetParent<Object> ()
      .SetGroupName ("Vlc")
      .AddConstructor<VlcCsmaCa> ()
    ;
    return tid;
  }

  VlcCsmaCa::VlcCsmaCa ()
  {
    // TODO-- make these into ns-3 attributes

    m_isSlotted = false;
    m_NB = 0;
    m_CW = 2;
    m_BLE = false;
    m_macMinBE = 3;
    m_macMaxBE = 5;
    m_macMaxCSMABackoffs = 4;
    m_aUnitBackoffPeriod = 20; //20 symbols
    m_random = CreateObject<UniformRandomVariable> ();
    m_BE = m_macMinBE;
    m_ccaRequestRunning = false;
  }

  VlcCsmaCa::~VlcCsmaCa ()
  {
    m_mac = 0;
  }

  void
  VlcCsmaCa::DoDispose ()
  {
    m_vlcMacStateCallback = MakeNullCallback< void, VlcMacState> ();
    Cancel ();
    m_mac = 0;
  }

  void
  VlcCsmaCa::SetMac (Ptr<VlcMac> mac)
  {
    m_mac = mac;
  }

  Ptr<VlcMac>
  VlcCsmaCa::GetMac (void) const
  {
    return m_mac;
  }

  void
  VlcCsmaCa::SetSlottedCsmaCa (void)
  {
    NS_LOG_FUNCTION (this);
    m_isSlotted = true;
  }

  void
  VlcCsmaCa::SetUnSlottedCsmaCa (void)
  {
    NS_LOG_FUNCTION (this);
    m_isSlotted = false;
  }

  bool
  VlcCsmaCa::IsSlottedCsmaCa (void) const
  {
    NS_LOG_FUNCTION (this);
    return (m_isSlotted);
  }

  bool
  VlcCsmaCa::IsUnSlottedCsmaCa (void) const
  {
    NS_LOG_FUNCTION (this);
    return (!m_isSlotted);
  }

  void
  VlcCsmaCa::SetMacMinBE (uint8_t macMinBE)
  {
    NS_LOG_FUNCTION (this << macMinBE);
    m_macMinBE = macMinBE;
  }

  uint8_t
  VlcCsmaCa::GetMacMinBE (void) const
  {
    NS_LOG_FUNCTION (this);
    return m_macMinBE;
  }

  void
  VlcCsmaCa::SetMacMaxBE (uint8_t macMaxBE)
  {
    NS_LOG_FUNCTION (this << macMaxBE);
    m_macMinBE = macMaxBE;
  }

  uint8_t
  VlcCsmaCa::GetMacMaxBE (void) const
  {
    NS_LOG_FUNCTION (this);
    return m_macMaxBE;
  }

  void
  VlcCsmaCa::SetMacMaxCSMABackoffs (uint8_t macMaxCSMABackoffs)
  {
    NS_LOG_FUNCTION (this << macMaxCSMABackoffs);
    m_macMaxCSMABackoffs = macMaxCSMABackoffs;
  }

  uint8_t
  VlcCsmaCa::GetMacMaxCSMABackoffs (void) const
  {
    NS_LOG_FUNCTION (this);
    return m_macMaxCSMABackoffs;
  }

  void
  VlcCsmaCa::SetUnitBackoffPeriod (uint64_t unitBackoffPeriod)
  {
    NS_LOG_FUNCTION (this << unitBackoffPeriod);
    m_aUnitBackoffPeriod = unitBackoffPeriod;
  }

  uint64_t
  VlcCsmaCa::GetUnitBackoffPeriod (void) const
  {
    NS_LOG_FUNCTION (this);
    return m_aUnitBackoffPeriod;
  }

  Time
  VlcCsmaCa::GetTimeToNextSlot (void) const
  {
    NS_LOG_FUNCTION (this);

    // TODO: Calculate the offset to the next slot.

    return Seconds (0);

  }
  void
  VlcCsmaCa::Start ()

  {
    NS_LOG_FUNCTION (this);
    m_NB = 0;
    if (IsSlottedCsmaCa ())
      {
        m_CW = 2;
        if (m_BLE)
          {
            m_BE = std::min (static_cast<uint8_t> (2), m_macMinBE);
          }
        else
          {
            m_BE = m_macMinBE;
          }
        //TODO: for slotted, locate backoff period boundary. i.e. delay to the next slot boundary
        Time backoffBoundary = GetTimeToNextSlot ();
        m_randomBackoffEvent = Simulator::Schedule (backoffBoundary, &VlcCsmaCa::RandomBackoffDelay, this);
      }
    else
      {
        m_BE = m_macMinBE;
        m_randomBackoffEvent = Simulator::ScheduleNow (&VlcCsmaCa::RandomBackoffDelay, this);
      }
    /*
    *  TODO: If using Backoff.cc (will need to modify Backoff::GetBackoffTime)
    *        Backoff.m_minSlots = 0;
    *        Backoff.m_ceiling = m_BE;
    *        Backoff.ResetBackoffTime(); //m_NB is same as m_numBackoffRetries in Backoff.h
    *        Backoff.m_maxRetries = macMaxCSMABackoffs;
    *        Backoff.m_slotTime = m_backoffPeriod;
    */
  }

  void
  VlcCsmaCa::Cancel ()
  {
    m_randomBackoffEvent.Cancel ();
    m_requestCcaEvent.Cancel ();
    m_canProceedEvent.Cancel ();
  }

  /*
   * Delay for backoff period in the range 0 to 2^BE -1 units
   * TODO: If using Backoff.cc (Backoff::GetBackoffTime) will need to be slightly modified
   */
  void
  VlcCsmaCa::RandomBackoffDelay ()
  {
    NS_LOG_FUNCTION (this);

    uint64_t upperBound = (uint64_t) pow (2, m_BE) - 1;
    uint64_t backoffPeriod;
    Time randomBackoff;
    uint64_t symbolRate;
    bool isData = false;


    symbolRate = (uint64_t) m_mac->GetPhy ()->GetDataOrSymbolRate (isData); //symbols per second
    backoffPeriod = (uint64_t)m_random->GetValue (0, upperBound+1); // num backoff periods
    randomBackoff = MicroSeconds (backoffPeriod * GetUnitBackoffPeriod () * 1000 * 1000 / symbolRate);

    if (IsUnSlottedCsmaCa ())
      {
        NS_LOG_LOGIC ("Unslotted:  requesting CCA after backoff of " << randomBackoff.GetMicroSeconds () << " us");
        m_requestCcaEvent = Simulator::Schedule (randomBackoff, &VlcCsmaCa::RequestCCA, this);
      }
    else
      {
        NS_LOG_LOGIC ("Slotted:  proceeding after backoff of " << randomBackoff.GetMicroSeconds () << " us");
        m_canProceedEvent = Simulator::Schedule (randomBackoff, &VlcCsmaCa::CanProceed, this);
      }
  }

  // TODO : Determine if transmission can be completed before end of CAP for the slotted csmaca
  //        If not delay to the next CAP
  void
  VlcCsmaCa::CanProceed ()
  {
    NS_LOG_FUNCTION (this);

    bool canProceed = true;

    if (m_BLE)
      {
      }
    else
      {
      }

    if (canProceed)
      {
        // TODO: For slotted, Perform CCA on backoff period boundary i.e. delay to next slot boundary
        Time backoffBoundary = GetTimeToNextSlot ();
        m_requestCcaEvent = Simulator::Schedule (backoffBoundary, &VlcCsmaCa::RequestCCA, this);
      }
    else
      {
        Time nextCap = Seconds (0);
        m_randomBackoffEvent = Simulator::Schedule (nextCap, &VlcCsmaCa::RandomBackoffDelay, this);
      }
  }

  void
  VlcCsmaCa::RequestCCA ()
  {
    NS_LOG_FUNCTION (this);
    m_ccaRequestRunning = true;
    m_mac->GetPhy ()->PlmeCcaRequest ();
  }

  /*
   * This function is called when the phy calls back after completing a PlmeCcaRequest
   */
  void
  VlcCsmaCa::PlmeCcaConfirm (VlcPhyEnumeration status)
  {
    NS_LOG_FUNCTION (this << status);

    // Only react on this event, if we are actually waiting for a CCA.
    // If the CSMA algorithm was canceled, we could still receive this event from
    // the PHY. In this case we ignore the event.
    if (m_ccaRequestRunning)
      {
        m_ccaRequestRunning = false;
        if (status == IEEE_802_15_7_PHY_IDLE)
          {
            if (IsSlottedCsmaCa ())
              {
                m_CW--;
                if (m_CW == 0)
                  {
                    // inform MAC channel is idle
                    if (!m_vlcMacStateCallback.IsNull ())
                      {
                        NS_LOG_LOGIC ("Notifying MAC of idle channel");
                        m_vlcMacStateCallback (CHANNEL_IDLE);
                      }
                  }
                else
                  {
                    NS_LOG_LOGIC ("Perform CCA again, m_CW = " << m_CW);
                    m_requestCcaEvent = Simulator::ScheduleNow (&VlcCsmaCa::RequestCCA, this); // Perform CCA again
                  }
              }
            else
              {
                // inform MAC, channel is idle
                if (!m_vlcMacStateCallback.IsNull ())
                  {
                    NS_LOG_LOGIC ("Notifying MAC of idle channel");
                    m_vlcMacStateCallback (CHANNEL_IDLE);
                  }
              }
          }
        else
          {
            if (IsSlottedCsmaCa ())
              {
                m_CW = 2;
              }
            m_BE = std::min (static_cast<uint16_t> (m_BE + 1), static_cast<uint16_t> (m_macMaxBE));
            m_NB++;
            if (m_NB > m_macMaxCSMABackoffs)
              {
                // no channel found so cannot send pkt
                NS_LOG_DEBUG ("Channel access failure");
                if (!m_vlcMacStateCallback.IsNull ())
                  {
                    NS_LOG_LOGIC ("Notifying MAC of Channel access failure");
                    m_vlcMacStateCallback (CHANNEL_ACCESS_FAILURE);
                  }
                return;
              }
            else
              {
                NS_LOG_DEBUG ("Perform another backoff; m_NB = " << static_cast<uint16_t> (m_NB));
                m_randomBackoffEvent = Simulator::ScheduleNow (&VlcCsmaCa::RandomBackoffDelay, this); //Perform another backoff (step 2)
              }
          }
      }
  }

  void
  VlcCsmaCa::SetVlcMacStateCallback (VlcMacStateCallback c)
  {
    NS_LOG_FUNCTION (this);
    m_vlcMacStateCallback = c;
  }

  int64_t
  VlcCsmaCa::AssignStreams (int64_t stream)
  {
    NS_LOG_FUNCTION (this);
    m_random->SetStream (stream);
    return 1;
  }

  uint8_t
  VlcCsmaCa::GetNB (void)
  {
    return m_NB;
  }


}
