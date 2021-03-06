/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors
 *and contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the
 *terms of the GNU General Public License as published by the Free Software
 *Foundation, either version 3 of the License, or (at your option) any later
 *version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY
 *WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 *A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see
 *<http://www.gnu.org/licenses/>.
 **/

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace std;

namespace ns3 {

int main( int argc, char *argv[] ) {
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault( "ns3::PointToPointNetDevice::DataRate",
                      StringValue( "10Mbps" ) );
  Config::SetDefault( "ns3::PointToPointChannel::Delay",
                      StringValue( "0ms" ) );
  Config::SetDefault( "ns3::DropTailQueue::MaxPackets", StringValue( "200" ) );

  // Read optional command-line parameters (e.g., enable visualizer with ./waf
  // --run=<> --visualize
  CommandLine cmd;
  cmd.Parse( argc, argv );

  NodeContainer nodes;
  nodes.Create( 11 );

  // // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install( nodes.Get( 0 ), nodes.Get( 1 ) );
  p2p.Install( nodes.Get( 0 ), nodes.Get( 5 ) );
  p2p.Install( nodes.Get( 0 ), nodes.Get( 10 ) );
  p2p.Install( nodes.Get( 1 ), nodes.Get( 2 ) );
  p2p.Install( nodes.Get( 1 ), nodes.Get( 3 ) );
  p2p.Install( nodes.Get( 2 ), nodes.Get( 3 ) );
  p2p.Install( nodes.Get( 3 ), nodes.Get( 4 ) );
  p2p.Install( nodes.Get( 4 ), nodes.Get( 5 ) );
  p2p.Install( nodes.Get( 5 ), nodes.Get( 6 ) );
  p2p.Install( nodes.Get( 6 ), nodes.Get( 7 ) );
  p2p.Install( nodes.Get( 6 ), nodes.Get( 10 ) );
  p2p.Install( nodes.Get( 7 ), nodes.Get( 8 ) );
  p2p.Install( nodes.Get( 8 ), nodes.Get( 9 ) );
  p2p.Install( nodes.Get( 9 ), nodes.Get( 10 ) );

  string strategy_name       = "SubPub";
  string zipf                = "0.8"; // 齐普夫参数
  string cache_size          = "130"; // 缓存大小
  string request_rate        = "20";  // 请求速率
  string average_update_time = "30";  // 更新时间
  string pit_store_size      = "126"; // PITCS表大小
  string update_factor =
      "1.0"; // 更新因子，0表示同一内容内容更新时间不变，1表示更新时间在内容到期后必变
  string experiment_time       = "120";
  string top_popularity_number = "130";

  string rate_trace = "rate-trace-" + strategy_name + "-" + zipf + "-" +
                      cache_size + "-" + request_rate + "-" +
                      average_update_time + "-" + pit_store_size + "-" +
                      update_factor + "-" + experiment_time + ".txt";
  string cs_trace = "cs-trace-" + strategy_name + "-" + zipf + "-" +
                    cache_size + "-" + request_rate + "-" +
                    average_update_time + "-" + pit_store_size + "-" +
                    update_factor + "-" + experiment_time + ".txt";

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes( true );
  ndnHelper.setCsSize( 0 );
  ndnHelper.SetOldContentStore( "ns3::ndn::cs::Lru", "MaxSize", cache_size );
  // ndnHelper.setCsSize(100);
  // ndnHelper.setPolicy("nfd::cs::lru")
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll( "/prefix",
                                         "/localhost/nfd/strategy/best-route" );
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();
  // Installing applications
  Ptr<Node> producer = nodes.Get( 0 );

  NodeContainer consumerNodes;
  for ( int i = 0; i < 11; i++ ) {
    consumerNodes.Add( nodes.Get( i ) );
  }

  // Consumer
  ndn::AppHelper consumerHelper( "ns3::ndn::ConsumerZipfMandelbrotKan" );
  consumerHelper.SetAttribute( "NumberOfContents", StringValue( "5000" ) );
  consumerHelper.SetAttribute( "q", StringValue( "0" ) );
  consumerHelper.SetAttribute( "s", StringValue( zipf ) );
  consumerHelper.SetAttribute( "Frequency", StringValue( request_rate ) );
  consumerHelper.SetPrefix( "/prefix" );
  consumerHelper.Install( consumerNodes );

  // Producer
  ndn::AppHelper producerHelper( "ns3::ndn::ProducerKanV2" );
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix( "/prefix" );
  producerHelper.SetAttribute( "PayloadSize", StringValue( "1024" ) );
  producerHelper.SetAttribute( "AverageUpdateTime",
                               StringValue( average_update_time ) );
  producerHelper.SetAttribute( "MaxPITStoreSize",
                               StringValue( pit_store_size ) );
  producerHelper.SetAttribute( "ExprimentTime",
                               StringValue( experiment_time ) );
  producerHelper.SetAttribute( "EligibleContentsNumber",
                               StringValue( top_popularity_number ) );
  // producerHelper.Install( nodes.Get( 4 ) ); // last node
  producerHelper.Install( producer );
  ndnGlobalRoutingHelper.AddOrigins( "/prefix", producer );
  // producerHelper.Install( producersNodes );
  // ndnGlobalRoutingHelper.AddOrigins( "/prefix", producersNodes );
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop( Seconds( stod( experiment_time ) ) );

  // ndn::L3RateTracer::InstallAll( rate_trace, Seconds( 1.0 ) );

  // ndn::CsTracer::InstallAll( cs_trace, Seconds( 1 ) );

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int main( int argc, char *argv[] ) { return ns3::main( argc, argv ); }
