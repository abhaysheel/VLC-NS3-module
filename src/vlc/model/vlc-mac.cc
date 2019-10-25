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
#include "vlc-mac.h"
#include "vlc-csmaca.h"
#include "vlc-mac-header.h"
#include "vlc-mac-trailer.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                   \
  std::clog << "[address " << m_shortAddress << "] ";

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("VlcMac");

NS_OBJECT_ENSURE_REGISTERED (VlcMac);

const uint32_t VlcMac::aMinMPDUOverhead = 9;

TypeId
VlcMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcMac")
    .SetParent<Object> ()
    .SetGroupName ("Vlc")
    .AddConstructor<VlcMac> ()
    .AddAttribute ("VpanId", "16-bit identifier of the associated VPAN",
                   UintegerValue (),
                   MakeUintegerAccessor (&VlcMac::m_macVpanId),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("MacTxEnqueue",
                     "Trace source indicating a packet has been "
                     "enqueued in the transaction queue",
                     MakeTraceSourceAccessor (&VlcMac::m_macTxEnqueueTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxDequeue",
                     "Trace source indicating a packet has was "
                     "dequeued from the transaction queue",
                     MakeTraceSourceAccessor (&VlcMac::m_macTxDequeueTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTx",
                     "Trace source indicating a packet has "
                     "arrived for transmission by this device",
                     MakeTraceSourceAccessor (&VlcMac::m_macTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxOk",
                     "Trace source indicating a packet has been "
                     "successfully sent",
                     MakeTraceSourceAccessor (&VlcMac::m_macTxOkTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxDrop",
                     "Trace source indicating a packet has been "
                     "dropped during transmission",
                     MakeTraceSourceAccessor (&VlcMac::m_macTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&VlcMac::m_macPromiscRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&VlcMac::m_macRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was received, "
                     "but dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&VlcMac::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&VlcMac::m_snifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PromiscSniffer",
                     "Trace source simulating a promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&VlcMac::m_promiscSnifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacStateValue",
                     "The state of Vlc Mac",
                     MakeTraceSourceAccessor (&VlcMac::m_vlcMacState),
                     "ns3::TracedValueCallback::VlcMacState")
    .AddTraceSource ("MacState",
                     "The state of Vlc Mac",
                     MakeTraceSourceAccessor (&VlcMac::m_macStateLogger),
                     "ns3::VlcMac::StateTracedCallback")
    .AddTraceSource ("MacSentPkt",
                     "Trace source reporting some information about "
                     "the sent packet",
                     MakeTraceSourceAccessor (&VlcMac::m_sentPktTrace),
                     "ns3::VlcMac::SentTracedCallback")
  ;
  return tid;
}

VlcMac::VlcMac ()
{
  m_vlcMacState = MAC_IDLE;
  ChangeMacState (MAC_IDLE);

  m_macRxOnWhenIdle = true;
  m_macVpanId = 0;
  m_associationStatus = ASSOCIATED;
  m_selfExt = Mac64Address::Allocate ();
  m_macPromiscuousMode = false;
  m_macMaxFrameRetries = 3;
  m_retransmission = 0;
  m_numCsmacaRetry = 0;
  m_txPkt = 0;

  Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable> ();
  uniformVar->SetAttribute ("Min", DoubleValue (0.0));
  uniformVar->SetAttribute ("Max", DoubleValue (255.0));
  m_macDsn = SequenceNumber8 (uniformVar->GetValue ());
  m_shortAddress = Mac16Address ("00:00");
}

VlcMac::~VlcMac ()
{
}

void
VlcMac::DoInitialize ()
{
  if (m_macRxOnWhenIdle)
    {
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_RX_ON);
    }
  else
    {
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_TRX_OFF);
    }

  Object::DoInitialize ();
}

void
VlcMac::DoDispose ()
{
  if (m_csmaCa != 0)
    {
      m_csmaCa->Dispose ();
      m_csmaCa = 0;
    }
  m_txPkt = 0;
  for (uint32_t i = 0; i < m_txQueue.size (); i++)
    {
      m_txQueue[i]->txQPkt = 0;
      delete m_txQueue[i];
    }
  m_txQueue.clear ();
  m_phy = 0;
  m_mcpsDataIndicationCallback = MakeNullCallback< void, McpsDataIndicationParams, Ptr<Packet> > ();
  m_mcpsDataConfirmCallback = MakeNullCallback< void, McpsDataConfirmParams > ();

  Object::DoDispose ();
}

bool
VlcMac::GetRxOnWhenIdle ()
{
  return m_macRxOnWhenIdle;
}

void
VlcMac::SetRxOnWhenIdle (bool rxOnWhenIdle)
{
  NS_LOG_FUNCTION (this << rxOnWhenIdle);
  m_macRxOnWhenIdle = rxOnWhenIdle;

  if (m_vlcMacState == MAC_IDLE)
    {
      if (m_macRxOnWhenIdle)
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_RX_ON);
        }
      else
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_TRX_OFF);
        }
    }
}

void
VlcMac::SetShortAddress (Mac16Address address)
{
  //NS_LOG_FUNCTION (this << address);
  m_shortAddress = address;
}

void
VlcMac::SetExtendedAddress (Mac64Address address)
{
  //NS_LOG_FUNCTION (this << address);
  m_selfExt = address;
}


Mac16Address
VlcMac::GetShortAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_shortAddress;
}

Mac64Address
VlcMac::GetExtendedAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_selfExt;
}
void
VlcMac::McpsDataRequest (McpsDataRequestParams params, Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  McpsDataConfirmParams confirmParams;
  confirmParams.m_msduHandle = params.m_msduHandle;

  // TODO: We need a drop trace for the case that the packet is too large or the request parameters are maleformed.
  //       The current tx drop trace is not suitable, because packets dropped using this trace carry the mac header
  //       and footer, while packets being dropped here do not have them.

  VlcMacHeader macHdr (VlcMacHeader::VLC_MAC_DATA, m_macDsn.GetValue ());
  m_macDsn++;

  if (p->GetSize () > VlcPhy::aMaxPhyPacketSize - aMinMPDUOverhead)
    {
      // Note, this is just testing maximum theoretical frame size per the spec
      // The frame could still be too large once headers are put on
      // in which case the phy will reject it instead
      NS_LOG_ERROR (this << " packet too big: " << p->GetSize ());
      confirmParams.m_status = IEEE_802_15_7_FRAME_TOO_LONG;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }

  if ((params.m_srcAddrMode == NO_VPANID_ADDR)
      && (params.m_dstAddrMode == NO_VPANID_ADDR))
    {
      NS_LOG_ERROR (this << " Can not send packet with no Address field" );
      confirmParams.m_status = IEEE_802_15_7_INVALID_ADDRESS;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }
  switch (params.m_srcAddrMode)
    {
    case NO_VPANID_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      //macHdr.SetNoVpanIdComp ();
      break;
    case ADDR_MODE_RESERVED:
      NS_ABORT_MSG ("Can not set source address type to ADDR_MODE_RESERVED. Aborting.");
      break;
    case SHORT_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      macHdr.SetSrcAddrFields (GetVpanId (), GetShortAddress ());
      break;
    case EXT_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      macHdr.SetSrcAddrFields (GetVpanId (), GetExtendedAddress ());
      break;
    default:
      NS_LOG_ERROR (this << " Can not send packet with incorrect Source Address mode = " << params.m_srcAddrMode);
      confirmParams.m_status = IEEE_802_15_7_INVALID_ADDRESS;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }
  switch (params.m_dstAddrMode)
    {
    case NO_VPANID_ADDR:
      macHdr.SetDstAddrMode (params.m_dstAddrMode);
      //macHdr.SetNoVpanIdComp ();
      break;
    case ADDR_MODE_RESERVED:
      NS_ABORT_MSG ("Can not set destination address type to ADDR_MODE_RESERVED. Aborting.");
      break;
    case SHORT_ADDR:
      macHdr.SetDstAddrMode (params.m_dstAddrMode);
      macHdr.SetDstAddrFields (params.m_dstVpanId, params.m_dstAddr);
      break;
    case EXT_ADDR:
      macHdr.SetDstAddrMode (params.m_dstAddrMode);
      macHdr.SetDstAddrFields (params.m_dstVpanId, params.m_dstExtAddr);
      break;
    default:
      NS_LOG_ERROR (this << " Can not send packet with incorrect Destination Address mode = " << params.m_dstAddrMode);
      confirmParams.m_status = IEEE_802_15_7_INVALID_ADDRESS;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }

  macHdr.SetSecDisable ();
  //extract the last 3 bits in TxOptions and map to macHdr
  int b0 = params.m_txOptions & TX_OPTION_ACK;
  int b1 = params.m_txOptions & TX_OPTION_GTS;
  int b2 = params.m_txOptions & TX_OPTION_INDIRECT;
  if (b0 == TX_OPTION_ACK)
    {
      // Set AckReq bit only if the destination is not the broadcast address.
      if (!(macHdr.GetDstAddrMode () == SHORT_ADDR && macHdr.GetShortDstAddr () == "ff:ff"))
        {
          macHdr.SetAckReq ();
        }
    }
  else if (b0 == 0)
    {
      macHdr.SetNoAckReq ();
    }
  else
    {
      confirmParams.m_status = IEEE_802_15_7_INVALID_PARAMETER;
      NS_LOG_ERROR (this << "Incorrect TxOptions bit 0 not 0/1");
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }

  //if is Slotted CSMA means its beacon enabled
  if (m_csmaCa->IsSlottedCsmaCa ())
    {
      if (b1 == TX_OPTION_GTS)
        {
          //TODO:GTS Transmission
        }
      else if (b1 == 0)
        {
          //TODO:CAP Transmission
        }
      else
        {
          NS_LOG_ERROR (this << "Incorrect TxOptions bit 1 not 0/1");
          confirmParams.m_status = IEEE_802_15_7_INVALID_PARAMETER;
          if (!m_mcpsDataConfirmCallback.IsNull ())
            {
              m_mcpsDataConfirmCallback (confirmParams);
            }
          return;
        }
    }
  else
    {
      if (b1 != 0)
        {
          NS_LOG_ERROR (this << "for non-beacon-enables VPAN, bit 1 should always be set to 0");
          confirmParams.m_status = IEEE_802_15_7_INVALID_PARAMETER;
          if (!m_mcpsDataConfirmCallback.IsNull ())
            {
              m_mcpsDataConfirmCallback (confirmParams);
            }
          return;
        }
    }

  if (b2 == TX_OPTION_INDIRECT)
    {
      //TODO :indirect tx
    }
  else if (b2 == 0)
    {
      //TODO :direct tx
    }
  else
    {
      NS_LOG_ERROR (this << "Incorrect TxOptions bit 2 not 0/1");
      confirmParams.m_status = IEEE_802_15_7_INVALID_PARAMETER;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }

  p->AddHeader (macHdr);

  VlcMacTrailer macTrailer;
  // Calculate FCS if the global attribute ChecksumEnable is set.
  if (Node::ChecksumEnabled ())
    {
      macTrailer.EnableFcs (true);
      macTrailer.SetFcs (p);
    }
  p->AddTrailer (macTrailer);

  m_macTxEnqueueTrace (p);

  TxQueueElement *txQElement = new TxQueueElement;
  txQElement->txQMsduHandle = params.m_msduHandle;
  txQElement->txQPkt = p;
  m_txQueue.push_back (txQElement);

  CheckQueue ();
}

void
VlcMac::CheckQueue ()
{
  NS_LOG_FUNCTION (this);

  // Pull a packet from the queue and start sending, if we are not already sending.
  if (m_vlcMacState == MAC_IDLE && !m_txQueue.empty () && m_txPkt == 0 && !m_setMacState.IsRunning ())
    {
      TxQueueElement *txQElement = m_txQueue.front ();
      m_txPkt = txQElement->txQPkt;
      m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_CSMA);
    }
}

void
VlcMac::SetCsmaCa (Ptr<VlcCsmaCa> csmaCa)
{
  m_csmaCa = csmaCa;
}

void
VlcMac::SetPhy (Ptr<VlcPhy> phy)
{
  m_phy = phy;
}

Ptr<VlcPhy>
VlcMac::GetPhy (void)
{
  return m_phy;
}

void
VlcMac::SetMcpsDataIndicationCallback (McpsDataIndicationCallback c)
{
  m_mcpsDataIndicationCallback = c;
}

void
VlcMac::SetMcpsDataConfirmCallback (McpsDataConfirmCallback c)
{
  m_mcpsDataConfirmCallback = c;
}

void
VlcMac::PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t wqi)
{
  NS_ASSERT (m_vlcMacState == MAC_IDLE || m_vlcMacState == MAC_ACK_PENDING || m_vlcMacState == MAC_CSMA);

  NS_LOG_FUNCTION (this << psduLength << p << (uint16_t)wqi);

  bool acceptFrame;

  Ptr<Packet> originalPkt = p->Copy (); // because we will strip headers

  m_promiscSnifferTrace (originalPkt);

  m_macPromiscRxTrace (originalPkt);
  // XXX no rejection tracing (to macRxDropTrace) being performed below

  VlcMacTrailer receivedMacTrailer;
  p->RemoveTrailer (receivedMacTrailer);
  if (Node::ChecksumEnabled ())
    {
      receivedMacTrailer.EnableFcs (true);
    }

  // level 1 filtering
  if (!receivedMacTrailer.CheckFcs (p))
    {
      m_macRxDropTrace (originalPkt);
    }
  else
    {
      VlcMacHeader receivedMacHdr;
      p->RemoveHeader (receivedMacHdr);

      McpsDataIndicationParams params;
      params.m_dsn = receivedMacHdr.GetSeqNum ();
      params.m_mpduLinkQuality = wqi;
      params.m_srcVpanId = receivedMacHdr.GetSrcVpanId ();
      params.m_srcAddrMode = receivedMacHdr.GetSrcAddrMode ();
      switch (params.m_srcAddrMode)
        {
        case SHORT_ADDR:
          params.m_srcAddr = receivedMacHdr.GetShortSrcAddr ();
          NS_LOG_DEBUG ("Packet from " << params.m_srcAddr);
          break;
        case EXT_ADDR:
          params.m_srcExtAddr = receivedMacHdr.GetExtSrcAddr ();
          NS_LOG_DEBUG ("Packet from " << params.m_srcExtAddr);
          break;
        default:
          break;
        }
      params.m_dstVpanId = receivedMacHdr.GetDstVpanId ();
      params.m_dstAddrMode = receivedMacHdr.GetDstAddrMode ();
      switch (params.m_dstAddrMode)
        {
        case SHORT_ADDR:
          params.m_dstAddr = receivedMacHdr.GetShortDstAddr ();
          NS_LOG_DEBUG ("Packet to " << params.m_dstAddr);
          break;
        case EXT_ADDR:
          params.m_dstExtAddr = receivedMacHdr.GetExtDstAddr ();
          NS_LOG_DEBUG ("Packet to " << params.m_dstExtAddr);
          break;
        default:
          break;
        }

      if (m_macPromiscuousMode)
        {
          //level 2 filtering
          if (!m_mcpsDataIndicationCallback.IsNull ())
            {
              NS_LOG_DEBUG ("promiscuous mode, forwarding up");
              m_mcpsDataIndicationCallback (params, p);
            }
          else
            {
              NS_LOG_ERROR (this << " Data Indication Callback not initialised");
            }
        }
      else
        {
          //level 3 frame filtering
          acceptFrame = (receivedMacHdr.GetType () != VlcMacHeader::VLC_MAC_RESERVED);

          if (acceptFrame)
            {
              acceptFrame = (receivedMacHdr.GetFrameVer () <= 1);
            }

          if (acceptFrame
              && (receivedMacHdr.GetDstAddrMode () > 1))
            {
              acceptFrame = receivedMacHdr.GetDstVpanId () == m_macVpanId
                || receivedMacHdr.GetDstVpanId () == 0xffff;
            }

          if (acceptFrame
              && (receivedMacHdr.GetDstAddrMode () == 2))
            {
              acceptFrame = receivedMacHdr.GetShortDstAddr () == m_shortAddress
                || receivedMacHdr.GetShortDstAddr () == Mac16Address ("ff:ff");        // check for broadcast addrs
            }

          if (acceptFrame
              && (receivedMacHdr.GetDstAddrMode () == 3))
            {
              acceptFrame = (receivedMacHdr.GetExtDstAddr () == m_selfExt);
            }

          if (acceptFrame
              && (receivedMacHdr.GetType () == VlcMacHeader::VLC_MAC_BEACON))
            {
              if (m_macVpanId == 0xffff)
                {
                  // TODO: Accept only if the frame version field is valid
                  acceptFrame = true;
                }
              else
                {
                  acceptFrame = receivedMacHdr.GetSrcVpanId () == m_macVpanId;
                }
            }

          if (acceptFrame
              && ((receivedMacHdr.GetType () == VlcMacHeader::VLC_MAC_DATA)
                  || (receivedMacHdr.GetType () == VlcMacHeader::VLC_MAC_COMMAND))
              && (receivedMacHdr.GetSrcAddrMode () > 1))
            {
              acceptFrame = receivedMacHdr.GetSrcVpanId () == m_macVpanId; // \todo need to check if PAN coord
            }

          if (acceptFrame)
            {
              m_macRxTrace (originalPkt);
              // \todo: What should we do if we receive a frame while waiting for an ACK?
              //        Especially if this frame has the ACK request bit set, should we reply with an ACK, possibly missing the pending ACK?

              // If the received frame is a frame with the ACK request bit set, we immediately send back an ACK.
              // If we are currently waiting for a pending ACK, we assume the ACK was lost and trigger a retransmission after sending the ACK.
              if ((receivedMacHdr.IsData () || receivedMacHdr.IsCommand ()) && receivedMacHdr.IsAckReq ()
                  && !(receivedMacHdr.GetDstAddrMode () == SHORT_ADDR && receivedMacHdr.GetShortDstAddr () == "ff:ff"))
                {
                  // If this is a data or mac command frame, which is not a broadcast,
                  // with ack req set, generate and send an ack frame.
                  // If there is a CSMA medium access in progress we cancel the medium access
                  // for sending the ACK frame. A new transmission attempt will be started
                  // after the ACK was send.
                  if (m_vlcMacState == MAC_ACK_PENDING)
                    {
                      m_ackWaitTimeout.Cancel ();
                      PrepareRetransmission ();
                    }
                  else if (m_vlcMacState == MAC_CSMA)
                    {
                      // \todo: If we receive a packet while doing CSMA/CA, should  we drop the packet because of channel busy,
                      //        or should we restart CSMA/CA for the packet after sending the ACK?
                      // Currently we simply restart CSMA/CA after sending the ACK.
                      m_csmaCa->Cancel ();
                    }
                  // Cancel any pending MAC state change, ACKs have higher priority.
                  m_setMacState.Cancel ();
                  ChangeMacState (MAC_IDLE);
                  m_setMacState = Simulator::ScheduleNow (&VlcMac::SendAck, this, receivedMacHdr.GetSeqNum ());
                }

              if (receivedMacHdr.IsData () && !m_mcpsDataIndicationCallback.IsNull ())
                {
                  // If it is a data frame, push it up the stack.
                  NS_LOG_DEBUG ("PdDataIndication():  Packet is for me; forwarding up");
                  m_mcpsDataIndicationCallback (params, p);
                }
              else if (receivedMacHdr.IsAcknowledgment () && m_txPkt && m_vlcMacState == MAC_ACK_PENDING)
                {
                  VlcMacHeader macHdr;
                  m_txPkt->PeekHeader (macHdr);
                  if (receivedMacHdr.GetSeqNum () == macHdr.GetSeqNum ())
                    {
                      m_macTxOkTrace (m_txPkt);
                      // If it is an ACK with the expected sequence number, finish the transmission
                      // and notify the upper layer.
                      m_ackWaitTimeout.Cancel ();
                      if (!m_mcpsDataConfirmCallback.IsNull ())
                        {
                          TxQueueElement *txQElement = m_txQueue.front ();
                          McpsDataConfirmParams confirmParams;
                          confirmParams.m_msduHandle = txQElement->txQMsduHandle;
                          confirmParams.m_status = IEEE_802_15_7_SUCCESS;
                          m_mcpsDataConfirmCallback (confirmParams);
                        }
                      RemoveFirstTxQElement ();
                      m_setMacState.Cancel ();
                      m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_IDLE);
                    }
                  else
                    {
                      // If it is an ACK with an unexpected sequence number, mark the current transmission as failed and start a retransmit. (cf 7.5.6.4.3)
                      m_ackWaitTimeout.Cancel ();
                      if (!PrepareRetransmission ())
                        {
                          m_setMacState.Cancel ();
                          m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_IDLE);
                        }
                      else
                        {
                          m_setMacState.Cancel ();
                          m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_CSMA);
                        }
                    }
                }
            }
          else
            {
              m_macRxDropTrace (originalPkt);
            }
        }
    }
}

void
VlcMac::SendAck (uint8_t seqno)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (seqno));

  NS_ASSERT (m_vlcMacState == MAC_IDLE);

  // Generate a corresponding ACK Frame.
  VlcMacHeader macHdr (VlcMacHeader::VLC_MAC_ACKNOWLEDGMENT, seqno);
  VlcMacTrailer macTrailer;
  Ptr<Packet> ackPacket = Create<Packet> (0);
  ackPacket->AddHeader (macHdr);
  // Calculate FCS if the global attribute ChecksumEnable is set.
  if (Node::ChecksumEnabled ())
    {
      macTrailer.EnableFcs (true);
      macTrailer.SetFcs (ackPacket);
    }
  ackPacket->AddTrailer (macTrailer);

  // Enqueue the ACK packet for further processing
  // when the transmitter is activated.
  m_txPkt = ackPacket;

  // Switch transceiver to TX mode. Proceed sending the Ack on confirm.
  ChangeMacState (MAC_SENDING);
  m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_TX_ON);
}

void
VlcMac::RemoveFirstTxQElement ()
{
  TxQueueElement *txQElement = m_txQueue.front ();
  Ptr<const Packet> p = txQElement->txQPkt;
  m_numCsmacaRetry += m_csmaCa->GetNB () + 1;

  Ptr<Packet> pkt = p->Copy ();
  VlcMacHeader hdr;
  pkt->RemoveHeader (hdr);
  if (hdr.GetShortDstAddr () != Mac16Address ("ff:ff"))
    {
      m_sentPktTrace (p, m_retransmission + 1, m_numCsmacaRetry);
    }

  txQElement->txQPkt = 0;
  delete txQElement;
  m_txQueue.pop_front ();
  m_txPkt = 0;
  m_retransmission = 0;
  m_numCsmacaRetry = 0;
  m_macTxDequeueTrace (p);
}

void
VlcMac::AckWaitTimeout (void)
{
  NS_LOG_FUNCTION (this);

  // TODO: If we are a PAN coordinator and this was an indirect transmission,
  //       we will not initiate a retransmission. Instead we wait for the data
  //       being extracted after a new data request command.
  if (!PrepareRetransmission ())
    {
      SetVlcMacState (MAC_IDLE);
    }
  else
    {
      SetVlcMacState (MAC_CSMA);
    }
}

bool
VlcMac::PrepareRetransmission (void)
{
  NS_LOG_FUNCTION (this);

  if (m_retransmission >= m_macMaxFrameRetries)
    {
      // Maximum number of retransmissions has been reached.
      // remove the copy of the packet that was just sent
      TxQueueElement *txQElement = m_txQueue.front ();
      m_macTxDropTrace (txQElement->txQPkt);
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          McpsDataConfirmParams confirmParams;
          confirmParams.m_msduHandle = txQElement->txQMsduHandle;
          confirmParams.m_status = IEEE_802_15_7_NO_ACK;
          m_mcpsDataConfirmCallback (confirmParams);
        }
      RemoveFirstTxQElement ();
      return false;
    }
  else
    {
      m_retransmission++;
      m_numCsmacaRetry += m_csmaCa->GetNB () + 1;
      // Start next CCA process for this packet.
      return true;
    }
}

void
VlcMac::PdDataConfirm (VlcPhyEnumeration status)
{
  NS_ASSERT (m_vlcMacState == MAC_SENDING);

  NS_LOG_FUNCTION (this << status << m_txQueue.size ());

  VlcMacHeader macHdr;
  m_txPkt->PeekHeader (macHdr);
  if (status == IEEE_802_15_7_PHY_SUCCESS)
    {
      if (!macHdr.IsAcknowledgment ())
        {
          // We have just send a regular data packet, check if we have to wait
          // for an ACK.
          if (macHdr.IsAckReq ())
            {
              // wait for the ack or the next retransmission timeout
              // start retransmission timer
              Time waitTime = MicroSeconds (GetMacAckWaitDuration () * 1000 * 1000 / m_phy->GetDataOrSymbolRate (false));
              NS_ASSERT (m_ackWaitTimeout.IsExpired ());
              m_ackWaitTimeout = Simulator::Schedule (waitTime, &VlcMac::AckWaitTimeout, this);
              m_setMacState.Cancel ();
              m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_ACK_PENDING);
              return;
            }
          else
            {
              m_macTxOkTrace (m_txPkt);
              // remove the copy of the packet that was just sent
              if (!m_mcpsDataConfirmCallback.IsNull ())
                {
                  McpsDataConfirmParams confirmParams;
                  NS_ASSERT_MSG (m_txQueue.size () > 0, "TxQsize = 0");
                  TxQueueElement *txQElement = m_txQueue.front ();
                  confirmParams.m_msduHandle = txQElement->txQMsduHandle;
                  confirmParams.m_status = IEEE_802_15_7_SUCCESS;
                  m_mcpsDataConfirmCallback (confirmParams);
                }
              RemoveFirstTxQElement ();
            }
        }
      else
        {
          // We have send an ACK. Clear the packet buffer.
          m_txPkt = 0;
        }
    }
  else if (status == IEEE_802_15_7_PHY_UNSPECIFIED)
    {

      if (!macHdr.IsAcknowledgment ())
        {
          NS_ASSERT_MSG (m_txQueue.size () > 0, "TxQsize = 0");
          TxQueueElement *txQElement = m_txQueue.front ();
          m_macTxDropTrace (txQElement->txQPkt);
          if (!m_mcpsDataConfirmCallback.IsNull ())
            {
              McpsDataConfirmParams confirmParams;
              confirmParams.m_msduHandle = txQElement->txQMsduHandle;
              confirmParams.m_status = IEEE_802_15_7_FRAME_TOO_LONG;
              m_mcpsDataConfirmCallback (confirmParams);
            }
          RemoveFirstTxQElement ();
        }
      else
        {
          NS_LOG_ERROR ("Unable to send ACK");
        }
    }
  else
    {
      // Something went really wrong. The PHY is not in the correct state for
      // data transmission.
      NS_FATAL_ERROR ("Transmission attempt failed with PHY status " << status);
    }

  m_setMacState.Cancel ();
  m_setMacState = Simulator::ScheduleNow (&VlcMac::SetVlcMacState, this, MAC_IDLE);
}

void
VlcMac::PlmeCcaConfirm (VlcPhyEnumeration status)
{
  NS_LOG_FUNCTION (this << status);
  // Direct this call through the csmaCa object
  m_csmaCa->PlmeCcaConfirm (status);
}
    // TODO : CHECK
//void
//VlcMac::PlmeEdConfirm (VlcPhyEnumeration status, uint8_t energyLevel)
//{
//  NS_LOG_FUNCTION (this << status << energyLevel);

//}

void
VlcMac::PlmeGetAttributeConfirm (VlcPhyEnumeration status,
                                    VlcPibAttributeIdentifier id,
                                    VlcPhyPibAttributes* attribute)
{
  NS_LOG_FUNCTION (this << status << id << attribute);
}

void
VlcMac::PlmeSetTRXStateConfirm (VlcPhyEnumeration status)
{
  NS_LOG_FUNCTION (this << status);

  if (m_vlcMacState == MAC_SENDING && (status == IEEE_802_15_7_PHY_TX_ON || status == IEEE_802_15_7_PHY_SUCCESS))
    {
      NS_ASSERT (m_txPkt);

      // Start sending if we are in state SENDING and the PHY transmitter was enabled.
      m_promiscSnifferTrace (m_txPkt);
      m_snifferTrace (m_txPkt);
      m_macTxTrace (m_txPkt);
      m_phy->PdDataRequest (m_txPkt->GetSize (), m_txPkt);
    }
  else if (m_vlcMacState == MAC_CSMA && (status == IEEE_802_15_7_PHY_RX_ON || status == IEEE_802_15_7_PHY_SUCCESS))
    {
      // Start the CSMA algorithm as soon as the receiver is enabled.
      m_csmaCa->Start ();
    }
  else if (m_vlcMacState == MAC_IDLE)
    {
      NS_ASSERT (status == IEEE_802_15_7_PHY_RX_ON || status == IEEE_802_15_7_PHY_SUCCESS || status == IEEE_802_15_7_PHY_TRX_OFF);
      // Do nothing special when going idle.
    }
  else if (m_vlcMacState == MAC_ACK_PENDING)
    {
      NS_ASSERT (status == IEEE_802_15_7_PHY_RX_ON || status == IEEE_802_15_7_PHY_SUCCESS);
    }
  else
    {
      // TODO: What to do when we receive an error?
      // If we want to transmit a packet, but switching the transceiver on results
      // in an error, we have to recover somehow (and start sending again).
      NS_FATAL_ERROR ("Error changing transceiver state");
    }
}

void
VlcMac::PlmeSetAttributeConfirm (VlcPhyEnumeration status,
                                    VlcPibAttributeIdentifier id)
{
  NS_LOG_FUNCTION (this << status << id);
}

void
VlcMac::SetVlcMacState (VlcMacState macState)
{
  NS_LOG_FUNCTION (this << "mac state = " << macState);

  McpsDataConfirmParams confirmParams;

  if (macState == MAC_IDLE)
    {
      ChangeMacState (MAC_IDLE);

      if (m_macRxOnWhenIdle)
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_RX_ON);
        }
      else
        {
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_TRX_OFF);
        }

      CheckQueue ();
    }
  else if (macState == MAC_ACK_PENDING)
    {
      ChangeMacState (MAC_ACK_PENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_RX_ON);
    }
  else if (macState == MAC_CSMA)
    {
      NS_ASSERT (m_vlcMacState == MAC_IDLE || m_vlcMacState == MAC_ACK_PENDING);

      ChangeMacState (MAC_CSMA);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_RX_ON);
    }
  else if (m_vlcMacState == MAC_CSMA && macState == CHANNEL_IDLE)
    {
      // Channel is idle, set transmitter to TX_ON
      ChangeMacState (MAC_SENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_7_PHY_TX_ON);
    }
  else if (m_vlcMacState == MAC_CSMA && macState == CHANNEL_ACCESS_FAILURE)
    {
      NS_ASSERT (m_txPkt);

      // cannot find a clear channel, drop the current packet.
      NS_LOG_DEBUG ( this << " cannot find clear channel");
      confirmParams.m_msduHandle = m_txQueue.front ()->txQMsduHandle;
      confirmParams.m_status = IEEE_802_15_7_CHANNEL_ACCESS_FAILURE;
      m_macTxDropTrace (m_txPkt);
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      // remove the copy of the packet that was just sent
      RemoveFirstTxQElement ();

      ChangeMacState (MAC_IDLE);
    }
}

VlcAssociationStatus
VlcMac::GetAssociationStatus (void) const
{
  return m_associationStatus;
}

void
VlcMac::SetAssociationStatus (VlcAssociationStatus status)
{
  m_associationStatus = status;
}

uint16_t
VlcMac::GetVpanId (void) const
{
  return m_macVpanId;
}

void
VlcMac::SetVpanId (uint16_t vpanId)
{
  m_macVpanId = vpanId;
}

void
VlcMac::ChangeMacState (VlcMacState newState)
{
  NS_LOG_LOGIC (this << " change vlc mac state from "
                     << m_vlcMacState << " to "
                     << newState);
  m_macStateLogger (m_vlcMacState, newState);
  m_vlcMacState = newState;
}

uint64_t
VlcMac::GetMacAckWaitDuration (void) const
{
  return m_csmaCa->GetUnitBackoffPeriod () + m_phy->aTurnaroundTime_RX_TX + m_phy->GetPhySHRDuration ()
         + ceil (6 * m_phy->GetPhySymbolsPerOctet ());
}

uint8_t
VlcMac::GetMacMaxFrameRetries (void) const
{
  return m_macMaxFrameRetries;
}

void
VlcMac::SetMacMaxFrameRetries (uint8_t retries)
{
  m_macMaxFrameRetries = retries;
}

} // namespace ns3
