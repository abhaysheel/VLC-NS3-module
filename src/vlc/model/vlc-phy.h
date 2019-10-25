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
#ifndef VLC_PHY_H
#define VLC_PHY_H

#include "vlc-interference-helper.h"
#include <ns3/spectrum-phy.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>
#include <ns3/event-id.h>

namespace ns3 {

class Packet;
class SpectrumValue;
class VlcErrorModel;
struct VlcSpectrumSignalParameters;
class MobilityModel;
class SpectrumChannel;
class SpectrumModel;
class AntennaModel;
class NetDevice;
class UniformRandomVariable;

//typedef struct
//{
//  double averagePower;    //!< Average measured power
//  Time lastUpdate;        //!< Last update time
//  Time measurementLength; //!< Total measuremement period
//} VlcEdPower;

typedef  struct
{
  double bitRate;    //!< bit rate
  double symbolRate; //!< symbol rate
} VlcPhyDataAndSymbolRates;

typedef  struct
{
  double shrPreamble; //!< Number of symbols for the SHR preamble
  //double shrSfd;      //!< Number of symbols for the SHR SFD
  double phrPhyHeader;         //!< Number of symbols for the PHR
  double phrHcs;       // TODO : Check
} VlcPhyPpduHeaderSymbolNumber;

typedef enum
{
  //IEEE_802_15_4_868MHZ_BPSK         = 0,
  //IEEE_802_15_4_915MHZ_BPSK         = 1,
  //IEEE_802_15_4_868MHZ_ASK          = 2,
  //IEEE_802_15_4_915MHZ_ASK          = 3,
  //IEEE_802_15_4_868MHZ_OQPSK        = 4,
  //IEEE_802_15_4_915MHZ_OQPSK        = 5,
  //IEEE_802_15_4_2_4GHZ_OQPSK        = 6,
  IEEE_802_15_7_4B6B_VPPM             =1,
  IEEE_802_15_7_8B10B_OOK             =2,
  IEEE_802_15_7_INVALID_PHY_OPTION  = 3
} VlcPhyOption;

typedef enum
{
  IEEE_802_15_7_PHY_BUSY  = 0x00,
  IEEE_802_15_7_PHY_BUSY_RX = 0x01,
  IEEE_802_15_7_PHY_BUSY_TX = 0x02,
  IEEE_802_15_7_PHY_FORCE_TRX_OFF = 0x03,
  IEEE_802_15_7_PHY_IDLE = 0x04,
  IEEE_802_15_7_PHY_INVALID_PARAMETER = 0x05,
  IEEE_802_15_7_PHY_RX_ON = 0x06,
  IEEE_802_15_7_PHY_SUCCESS = 0x07,
  IEEE_802_15_7_PHY_TRX_OFF = 0x08,
  IEEE_802_15_7_PHY_TX_ON = 0x09,
  IEEE_802_15_7_PHY_UNSUPPORTED_ATTRIBUTE = 0xa,
  //IEEE_802_15_7_READ_ONLY = 0X0b
  IEEE_802_15_7_PHY_UNSPECIFIED = 0xc // all cases not covered by ieee802.15.4
} VlcPhyEnumeration;

namespace TracedValueCallback
{

  typedef void (* VlcPhyEnumeration)(VlcPhyEnumeration oldValue,
                                     VlcPhyEnumeration newValue);
} // namespace TracedValueCallback

typedef enum
{
  phyCurrentChannel = 0x00,
  phyCCAMode = 0x01,
  phyDim = 0x02,
  phyUseExtendedMode = 0x03,
  phyColorFunction = 0x04,
  phyBlinkingNotificationFrequency = 0x05,
} VlcPibAttributeIdentifier;

typedef struct
{
  uint8_t phyCurrentChannel;
  //uint32_t phyChannelsSupported[32];
  //uint8_t phyTransmitPower;
  double phyCCAMode;
  uint32_t phyDim;
  uint8_t phyUseExtendedMode;
  uint32_t phyColorFunction[256][3];
  uint32_t phyBlinkingNotificationFrequency;

} VlcPhyPibAttributes;

typedef Callback<void, uint32_t, Ptr<Packet>, uint8_t > PdDataIndicationCallback;

typedef Callback<void, VlcPhyEnumeration > PdDataConfirmCallback;

typedef Callback<void, VlcPhyEnumeration > PlmeCcaConfirmCallback;

/**
* \ingroup lr-wpan
*
* This method implements the PD SAP: PlmeEdConfirm
*
* @param status the status of ED
* @param energyLevel the energy level of ED
*/
//typedef Callback< void, VlcPhyEnumeration,uint8_t > PlmeEdConfirmCallback;

typedef Callback< void, VlcPhyEnumeration,
                 VlcPibAttributeIdentifier,
                 VlcPhyPibAttributes* > PlmeGetAttributeConfirmCallback;

typedef Callback< void, VlcPhyEnumeration > PlmeSetTRXStateConfirmCallback;

typedef Callback< void, VlcPhyEnumeration,
                 VlcPibAttributeIdentifier > PlmeSetAttributeConfirmCallback;

/**
* \ingroup lr-wpan
*
* Make VlcPhy a SpectrumPhy so we can enable the eventual modeling of
* device interference
*/
class VlcPhy : public SpectrumPhy
{

public:

 static TypeId GetTypeId (void);

 static const uint32_t aMaxPhyPacketSize;

 /**
  * The turnaround time for switching the transceiver from RX to TX or vice
  * versa.
  * See Table 22 in section 6.4.1 of IEEE 802.15.4-2006
  */
 static const uint32_t aTurnaroundTime_TX_RX;

 static const uint32_t aTurnaroundTime_RX_TX;

 VlcPhy (void);
 virtual ~VlcPhy (void);

 // inherited from SpectrumPhy
 void SetMobility (Ptr<MobilityModel> m);
 Ptr<MobilityModel> GetMobility (void);
 void SetChannel (Ptr<SpectrumChannel> c);

 /**
  * Get the currently attached channel.
  *
  * \return the channel
  */

 Ptr<SpectrumChannel> GetChannel (void);
 void SetDevice (Ptr<NetDevice> d);
 Ptr<NetDevice> GetDevice (void) const;

 /**
  * Set the attached antenna.
  *
  * \param a the antenna
  */
  // TODO: CHECK
 void SetAntenna (Ptr<AntennaModel> a);
 Ptr<AntennaModel> GetRxAntenna (void);
 virtual Ptr<const SpectrumModel> GetRxSpectrumModel (void) const;

 void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);

 void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);

 Ptr<const SpectrumValue> GetNoisePowerSpectralDensity (void);

 /**
   * Notify the SpectrumPhy instance of an incoming waveform.
   *
   * @param params the SpectrumSignalParameters associated with the incoming waveform
   */
 virtual void StartRx (Ptr<SpectrumSignalParameters> params);

 void PdDataRequest (const uint32_t psduLength, Ptr<Packet> p);

 void PlmeCcaRequest (void);

 //void PlmeEdRequest (void);

 void PlmeGetAttributeRequest (VlcPibAttributeIdentifier id);

 void PlmeSetTRXStateRequest (VlcPhyEnumeration state);

 void PlmeSetAttributeRequest (VlcPibAttributeIdentifier id, VlcPhyPibAttributes* attribute);

 void SetPdDataIndicationCallback (PdDataIndicationCallback c);

 void SetPdDataConfirmCallback (PdDataConfirmCallback c);

 void SetPlmeCcaConfirmCallback (PlmeCcaConfirmCallback c);

// TODO : CHECK
// void SetPlmeEdConfirmCallback (PlmeEdConfirmCallback c);

 void SetPlmeGetAttributeConfirmCallback (PlmeGetAttributeConfirmCallback c);

 void SetPlmeSetTRXStateConfirmCallback (PlmeSetTRXStateConfirmCallback c);

 void SetPlmeSetAttributeConfirmCallback (PlmeSetAttributeConfirmCallback c);

 double GetDataOrSymbolRate (bool isData);

 void SetErrorModel (Ptr<VlcErrorModel> e);

 Ptr<VlcErrorModel> GetErrorModel (void) const;

 uint64_t GetPhySHRDuration (void) const;

 double GetPhySymbolsPerOctet (void) const;

 int64_t AssignStreams (int64_t stream);

 typedef void (* StateTracedCallback)
   (Time time, VlcPhyEnumeration oldState, VlcPhyEnumeration newState);

protected:
 /**
  * The data and symbol rates for the different PHY options.
  * See Table 2 in section 6.1.2 IEEE 802.15.4-2006
  */
 static const VlcPhyDataAndSymbolRates dataSymbolRates[2];
 /**
  * The preamble, SFD, and PHR lengths in symbols for the different PHY options.
  * See Table 19 and Table 20 in section 6.3 IEEE 802.15.4-2006
  */
 static const VlcPhyPpduHeaderSymbolNumber ppduHeaderSymbolNumbers[2];

private:
 /**
  * The second is true if the first is flagged as error/invalid.
  */
 typedef std::pair<Ptr<Packet>, bool>  PacketAndStatus;

 // Inherited from Object.
 virtual void DoDispose (void);

 /**
  * Change the PHY state to the given new state, firing the state change trace.
  *
  * \param newState the new state
  */
 void ChangeTrxState (VlcPhyEnumeration newState);

 /**
  * Configure the PHY option according to the current channel and channel page.
  * See IEEE 802.15.4-2006, section 6.1.2, Table 2.
  */
 void SetMyPhyOption (void);

 /**
  * Get the currently configured PHY option.
  * See IEEE 802.15.4-2006, section 6.1.2, Table 2.
  *
  * \return the PHY option
  */
 VlcPhyOption GetMyPhyOption (void);

 void EndTx (void);

 void CheckInterference (void);

 void EndRx (Ptr<SpectrumSignalParameters> params);

 void CancelEd (VlcPhyEnumeration state);

 void EndEd (void);

 void EndCca (void);

 void EndSetTRXState (void);

 Time CalculateTxTime (Ptr<const Packet> packet);

 Time GetPpduHeaderTxTime (void);

 bool ChannelSupported (uint8_t channel);

 bool PhyIsBusy (void) const;

 // Trace sources
 /**
  * The trace source fired when a packet begins the transmission process on
  * the medium.
  *
  * \see class CallBackTraceSource
  */
 TracedCallback<Ptr<const Packet> > m_phyTxBeginTrace;

 TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;

 TracedCallback<Ptr<const Packet> > m_phyTxDropTrace;

 TracedCallback<Ptr<const Packet> > m_phyRxBeginTrace;

 TracedCallback<Ptr<const Packet>, double > m_phyRxEndTrace;

 TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

 TracedCallback<Time, VlcPhyEnumeration, VlcPhyEnumeration> m_trxStateLogger;

 /**
  * The mobility model used by the PHY.
  */
 Ptr<MobilityModel> m_mobility;

 /**
  * The configured net device.
  */
 Ptr<NetDevice> m_device;

 /**
  * The channel attached to this transceiver.
  */
 Ptr<SpectrumChannel> m_channel;

 /**
  * The antenna used by the transceiver.
  */
  // TODO : Check
 Ptr<AntennaModel> m_antenna;

 /**
  * The transmit power spectral density.
  */
 Ptr<SpectrumValue> m_txPsd;

 /**
  * The spectral density for for the noise.
  */
  // TODO : Check
// Ptr<const SpectrumValue> m_noise;

 /**
  * The error model describing the bit and packet error rates.
  */
 Ptr<VlcErrorModel> m_errorModel;

 /**
  * The current PHY PIB attributes.
  */
 VlcPhyPibAttributes m_phyPIBAttributes;

 TracedValue<VlcPhyEnumeration> m_trxState;

 VlcPhyEnumeration m_trxStatePending;

 PdDataIndicationCallback m_pdDataIndicationCallback;

 PdDataConfirmCallback m_pdDataConfirmCallback;

 PlmeCcaConfirmCallback m_plmeCcaConfirmCallback;

// TODO : CHECK
// PlmeEdConfirmCallback m_plmeEdConfirmCallback;


 PlmeGetAttributeConfirmCallback m_plmeGetAttributeConfirmCallback;

 PlmeSetTRXStateConfirmCallback m_plmeSetTRXStateConfirmCallback;

 PlmeSetAttributeConfirmCallback m_plmeSetAttributeConfirmCallback;

 /**
  * The currently configured PHY type.
  */
 VlcPhyOption m_phyOption;

 /**
  * Helper value for tracking the average power during ED.
  */
// TODO: CHECK
// VlcEdPower m_edPower;

 /**
  * Helper value for the peak power value during CCA.
  */
 double m_ccaPeakPower;

 /**
  * The receiver sensitivity.
  */
 double m_rxSensitivity;

 /**
  * The accumulated signals currently received by the transceiver, including
  * the signal of a possibly received packet, as well as all signals
  * considered noise.
  */
 Ptr<VlcInterferenceHelper> m_signal;

 /**
  * Timestamp of the last calculation of the PER of a packet currently received.
  */
 Time m_rxLastUpdate;

 /**
  * Statusinformation of the currently received packet. The first parameter
  * contains the frame, as well the signal power of the frame. The second
  * parameter is set to false, if the frame is either invalid or destroyed
  * due to interference.
  */
 std::pair<Ptr<VlcSpectrumSignalParameters>, bool>  m_currentRxPacket;

 /**
  * Statusinformation of the currently transmitted packet. The first parameter
  * contains the frame. The second parameter is set to false, if the frame not
  * completely transmitted, in the event of a force transceiver switch, for
  * example.
  */
 PacketAndStatus m_currentTxPacket;

 /**
  * Scheduler event of a currently running CCA request.
  */
 EventId m_ccaRequest;

 /**
  * Scheduler event of a currently running ED request.
  */
 // TODO : CHECK
 //EventId m_edRequest;

 /**
  * Scheduler event of a currently running deferred transceiver state switch.
  */
 EventId m_setTRXState;

 /**
  * Scheduler event of a currently running data transmission request.
  */
 EventId m_pdDataRequest;

 /**
  * Uniform random variable stream.
  */
 Ptr<UniformRandomVariable> m_random;
};

} // namespace ns3

#endif /* VLC_PHY_H */
