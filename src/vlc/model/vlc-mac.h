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
 * Authors:
 *  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#ifndef VLC_MAC_H
#define VLC_MAC_H

#include <ns3/object.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/sequence-number.h>
#include <ns3/vlc-phy.h>
#include <ns3/event-id.h>
#include <deque>

namespace ns3 {

class Packet;
class VlcCsmaCa;

// TX-OPTIONS
typedef enum
{
  TX_OPTION_NONE = 0,    //!< TX_OPTION_NONE
  TX_OPTION_ACK = 1,     //!< TX_OPTION_ACK
  TX_OPTION_GTS = 2,     //!< TX_OPTION_GTS
  TX_OPTION_INDIRECT = 4 //!< TX_OPTION_INDIRECT
} VlcTxOption;

// MAC STATES
typedef enum
{
  MAC_IDLE,              //!< MAC_IDLE
  MAC_CSMA,              //!< MAC_CSMA
  MAC_SENDING,           //!< MAC_SENDING
  MAC_ACK_PENDING,       //!< MAC_ACK_PENDING
  CHANNEL_ACCESS_FAILURE,//!< CHANNEL_ACCESS_FAILURE
  CHANNEL_IDLE,          //!< CHANNEL_IDLE
  SET_PHY_TX_ON          //!< SET_PHY_TX_ON
} VlcMacState;

namespace TracedValueCallback {

  typedef void(* VlcMacState)(VlcMacState oldValue,
                              VlcMacState newValue);
} // namespace TracedValueCallback

typedef enum
{
  NO_VPANID_ADDR = 0,
  ADDR_MODE_RESERVED = 1,
  SHORT_ADDR = 2,
  EXT_ADDR = 3
} VlcAddressMode;

typedef enum
{
  ASSOCIATED = 0,
  VPAN_AT_CAPACITY = 1,
  VPAN_ACCESS_DENIED = 2,
  RESERVED = 0X03,
  RESERVED_MAC_PREMITIVE = 0Xff
} VlcAssociationStatus;

typedef enum
{
  IEEE_802_15_7_SUCCESS                = 0,
  IEEE_802_15_7_TRANSACTION_OVERFLOW   = 1,
  IEEE_802_15_7_TRANSACTION_EXPIRED    = 2,
  IEEE_802_15_7_CHANNEL_ACCESS_FAILURE = 3,
  IEEE_802_15_7_INVALID_ADDRESS        = 4,
  IEEE_802_15_7_INVALID_GTS            = 5,
  IEEE_802_15_7_NO_ACK                 = 6,
  IEEE_802_15_7_COUNTER_ERROR          = 7,
  IEEE_802_15_7_FRAME_TOO_LONG         = 8,
  IEEE_802_15_7_UNAVAILABLE_KEY        = 9,
  IEEE_802_15_7_UNSUPPORTED_SECURITY   = 10,
  IEEE_802_15_7_INVALID_PARAMETER      = 11
} VlcMcpsDataConfirmStatus;

struct McpsDataRequestParams
{
  McpsDataRequestParams ()
    : m_srcAddrMode (SHORT_ADDR),
      m_dstAddrMode (SHORT_ADDR),
      m_dstVpanId (0),
      m_dstAddr (),
      //m_msduLength
      //m_msdu
      m_msduHandle (0),
      m_txOptions (0)
  {
  }
  VlcAddressMode m_srcAddrMode;    //!< Source address mode
  VlcAddressMode m_dstAddrMode; //!< Destination address mode
  uint16_t m_dstVpanId;             //!< Destination VPAN identifier
  Mac16Address m_dstAddr;          //!< Destination address
  Mac64Address m_dstExtAddr;       //!< Destination extended address
  uint8_t m_msduHandle;            //!< MSDU handle
  uint8_t m_txOptions;             //!< Tx Options (bitfield)
};

struct McpsDataConfirmParams
{
  uint8_t m_msduHandle;
  VlcMcpsDataConfirmStatus m_status;
};

struct McpsDataIndicationParams
{
  uint8_t m_srcAddrMode;  //!< Source address mode
  uint16_t m_srcVpanId;    //!< Source VPAN identifier
  Mac16Address m_srcAddr; //!< Source address
  Mac64Address m_srcExtAddr;  //!< Source extended address
  uint8_t m_dstAddrMode;  //!< Destination address mode
  uint16_t m_dstVpanId;    //!< Destination VPAN identifier
  Mac16Address m_dstAddr; //!< Destination address
  Mac64Address m_dstExtAddr;  //!< Destination extended address
  uint8_t m_mpduLinkQuality;  //!< LQI value measured during reception of the MPDU
  uint8_t m_dsn;          //!< The DSN of the received data frame
};

typedef Callback<void, McpsDataConfirmParams> McpsDataConfirmCallback;

typedef Callback<void, McpsDataIndicationParams, Ptr<Packet> >
    McpsDataIndicationCallback;

class VlcMac : public Object
{
public:

  static TypeId GetTypeId (void);

  static const uint32_t aMinMPDUOverhead;

  VlcMac (void);
  virtual ~VlcMac (void);

  bool GetRxOnWhenIdle (void);

  void SetRxOnWhenIdle (bool rxOnWhenIdle);

  void SetShortAddress (Mac16Address address);

  Mac16Address GetShortAddress (void) const;

  void SetExtendedAddress (Mac64Address address);

  Mac64Address GetExtendedAddress (void) const;

  void SetVpanId (uint16_t vpanId);

  uint16_t GetVpanId (void)  const;

  void McpsDataRequest (McpsDataRequestParams params, Ptr<Packet> p);

  void SetCsmaCa (Ptr<VlcCsmaCa> csmaCa);

  void SetPhy (Ptr<VlcPhy> phy);

  Ptr<VlcPhy> GetPhy (void);

  void SetMcpsDataIndicationCallback (McpsDataIndicationCallback c);

  void SetMcpsDataConfirmCallback (McpsDataConfirmCallback c);

  void PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t wqi);

  void PdDataConfirm (VlcPhyEnumeration status);

  void PlmeCcaConfirm (VlcPhyEnumeration status);
    // TODO : CHECK
//  void PlmeEdConfirm (VlcPhyEnumeration status, uint8_t energyLevel);

  void PlmeGetAttributeConfirm (VlcPhyEnumeration status,
                                VlcPibAttributeIdentifier id,
                                VlcPhyPibAttributes* attribute);

  void PlmeSetTRXStateConfirm (VlcPhyEnumeration status);

  void PlmeSetAttributeConfirm (VlcPhyEnumeration status,
                                VlcPibAttributeIdentifier id);

  void SetVlcMacState (VlcMacState macState);

  VlcAssociationStatus GetAssociationStatus (void) const;

  void SetAssociationStatus (VlcAssociationStatus status);

  //MAC SUBLAYER CONSTANTS - START
  uint64_t m_aBaseSlotDuration;

  uint64_t m_aNumSuperframeSlots;

  uint64_t m_aBaseSuperFrameDuration;
  //MAC SUBLAYER CONSTANTS - END


  // MAC PIB ATTRIBUTES - START
  uint64_t m_macBeaconTxTime;

  uint64_t m_macSyncSymbolOffset;  //PLEASE CHECK

  uint64_t m_macBeaconOrder;

  uint64_t m_macSupeframeOrder;

  uint64_t m_macPromiscuousMode;   //PLEASE CHECK

  uint16_t m_macVpanId;

  SequenceNumber8 m_macDsn;

  uint8_t m_macMaxFrameRetries;

  bool m_macRxOnWhenIdle;

  uint64_t GetMacAckWaitDuration (void) const;

  uint8_t GetMacMaxFrameRetries (void) const;

  void SetMacMaxFrameRetries (uint8_t retries);

  typedef void (* SentTracedCallback)
    (Ptr<const Packet> packet, uint8_t retries, uint8_t backoffs);

  typedef void (* StateTracedCallback)
    (VlcMacState olsState, VlcMacState newState);

protected:
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

private:

  struct TxQueueElement
  {
    uint8_t txQMsduHandle;
    Ptr<Packet> txQPkt;
  };

  void SendAck (uint8_t seqno);

  void RemoveFirstTxQElement ();

  void ChangeMacState (VlcMacState newState);

  void AckWaitTimeout (void);

  bool PrepareRetransmission (void);

  void CheckQueue (void);

  TracedCallback<Ptr<const Packet>, uint8_t, uint8_t > m_sentPktTrace;

  TracedCallback<Ptr<const Packet> > m_macTxEnqueueTrace;

  TracedCallback<Ptr<const Packet> > m_macTxDequeueTrace;

  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  TracedCallback<Ptr<const Packet> > m_macTxOkTrace;

  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  TracedCallback<VlcMacState, VlcMacState> m_macStateLogger;

  Ptr<VlcPhy> m_phy;

  Ptr<VlcCsmaCa> m_csmaCa;

  McpsDataIndicationCallback m_mcpsDataIndicationCallback;

  McpsDataConfirmCallback m_mcpsDataConfirmCallback;

  TracedValue<VlcMacState> m_vlcMacState;

  VlcAssociationStatus m_associationStatus;

  Ptr<Packet> m_txPkt;

  Mac16Address m_shortAddress;

  Mac64Address m_selfExt;

  std::deque<TxQueueElement*> m_txQueue;

  uint8_t m_retransmission;

  uint8_t m_numCsmacaRetry;

  EventId m_ackWaitTimeout;

  EventId m_setMacState;
};

} // namespace ns3

#endif /* VLC_MAC_H */
