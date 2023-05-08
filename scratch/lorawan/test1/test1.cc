/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("SimpleLorawanNetworkExample");

void deviceSend(int device, NodeContainer endDevices, Ptr<Packet> pkt)
{
  std::cout << "Function deviceSend called with arguments: device: " << device << ", endDevices: <>, pkt: " << pkt << std::endl;

  Ptr<Node> node = endDevices.Get (device);
  Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();

  NS_LOG_INFO ("Device: " << device << " - LoraNetDevice:" << loraNetDevice);

  loraNetDevice->Send(pkt);
}

int main (int argc, char *argv[])
{

  // Set up logging
  LogComponentEnable ("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
  // LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  // LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LorawanMacHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSenderHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  // LogComponentEnable ("LorawanMacHeader", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  /************************
  *  Create the channel  *
  ************************/

  NS_LOG_INFO ("Creating the channel...");

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  allocator->Add (Vector (1000,0,0));
  allocator->Add (Vector (0,0,0));
  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();


  /************************
  *  Crafting Packet  *
  ************************/

  int packetSize = 10;

  Ptr<Packet> pkt = Create<Packet> (packetSize);

  // create a LoRaPhyHeader
  // LoRaPhyHeader phyHeader;
  // phyHeader.SetMType (LoRaMacHeader::UNCONFIRMED_DATA_UP);
  // phyHeader.SetMajor (0);
  // phyHeader.SetRFU (0);

  // // add the LoRaPhyHeader to the packet
  // pkt->AddHeader (phyHeader);

  LoraFrameHeader frameHdr = LoraFrameHeader ();
  frameHdr.SetAsUplink ();
  frameHdr.SetFPort (1);
  frameHdr.SetAddress (LoraDeviceAddress ());
  frameHdr.SetAdr (0);
  frameHdr.SetAdrAckReq (0);
  frameHdr.SetFCnt (0);
  pkt->AddHeader (frameHdr);

  NS_LOG_INFO ("Packet to send...: " << pkt);


  /************************
  *  Create End Devices  *
  ************************/

  NS_LOG_INFO ("Creating the end device...");

  // Create a set of nodes
  NodeContainer endDevices;
  endDevices.Create (2);

  // Assign a mobility model to the node
  mobility.Install (endDevices);

  // Create a LoraDeviceAddressGenerator
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId,nwkAddr);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  macHelper.SetAddressGenerator (addrGen);
  macHelper.SetRegion (LorawanMacHelper::EU);
  helper.Install (phyHelper, macHelper, endDevices);


  // Set message type (Default is unconfirmed)
  Ptr<LorawanMac> edMac1 = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
  Ptr<ClassAEndDeviceLorawanMac> edLorawanMac1 = edMac1->GetObject<ClassAEndDeviceLorawanMac> ();
  edLorawanMac1->SetMType (LorawanMacHeader::CONFIRMED_DATA_UP);


  /*********************
  *  Create Gateways  *
  *********************/

  NS_LOG_INFO ("Creating the gateway...");
  NodeContainer gateways;
  gateways.Create (1);

  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // Set spreading factors up
  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

  /*********************************************
  *  Install applications on the end devices  *
  *********************************************/

  // OneShotSenderHelper oneShotSenderHelper;
  // oneShotSenderHelper.SetSendTime (Seconds (2));
  // oneShotSenderHelper.Install (endDevices.Get (0));
  // oneShotSenderHelper.SetSendTime (Seconds (4));
  // oneShotSenderHelper.Install (endDevices.Get (0));


  /*********************************************
  *  Scheduling Packets Transmission  *
  *********************************************/

  double delayInSeconds = 0.0;
  int device = 0;

  LoraFrameHeader loraHeader; // create a LoraFrameHeader object to hold the extracted header
  pkt->PeekHeader(loraHeader); // extract the LoraFrameHeader from the packet using PeekHeader method
  // loraHeader.SetFCnt(1);
  // loraHeader.SetFCnt(0);

  std::cout << "Device " << device << " : The counter before sendng it is: " << loraHeader.GetFCnt() << std::endl;

  Simulator::Schedule(Seconds(delayInSeconds), &deviceSend, device, endDevices, pkt);

  delayInSeconds = 3.0;
  device = 1;

  Simulator::Schedule(Seconds(delayInSeconds), &deviceSend, device, endDevices, pkt);


  /******************
   * Set Data Rates *
   ******************/
  // std::vector<int> sfQuantity (6);
  // sfQuantity = macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);


  ////////////
  // Create NS
  ////////////

  NodeContainer networkServers;
  networkServers.Create (1);

  // Install the NetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.Install (networkServers);

  // Install the Forwarder application on the gateways
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);


  /****************
  *  Simulation  *
  ****************/

  Simulator::Stop (Hours (2));

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
