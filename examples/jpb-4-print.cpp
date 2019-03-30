#include <iostream>
/* core module and required helpers for example */
#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
/* required for use of CS (from NFD) */
#include "ns3/ndnSIM/NFD/core/algorithm.hpp"
#include "ns3/ndnSIM/NFD/core/logger.hpp"
#include "ns3/ndnSIM/NFD/daemon/table/cs-policy-priority-fifo.hpp"
#include "ns3/ndnSIM/NFD/daemon/table/cs.hpp"
#include "ns3/ndnSIM/ndn-cxx/data.hpp"
#include "umd/JPBPrintContent.hpp"
//#include "JPBPolicy.hpp"
//#include <memory>  //for the unique_ptr and make_unique
#include <array>
#include <string>
using namespace std;

template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

namespace ns3
{
int main(int argc, char *argv[])
{
	/* setting default parameters for point to point links & channels */
	Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
	Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
	//Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));
	/* read optional command line params */
	CommandLine cmd;
	cmd.Parse(argc, argv);
	/*
	 * Create the node
	 * through the node container
	 */
	NodeContainer nodes;
	nodes.Create(4); /* [consumer] -- [router] -- [producer] */
					 /*
					  * [consumer] ----^
					  * connect nodes (create channels)
					  */
	PointToPointHelper p2p;
	p2p.Install(nodes.Get(0), nodes.Get(2));
	p2p.Install(nodes.Get(1), nodes.Get(2));
	p2p.Install(nodes.Get(2), nodes.Get(3));
	/* install the NDN stack on the node */
	ndn::StackHelper ndnHelper;
	ndnHelper.SetDefaultRoutes(true);
	ndnHelper.InstallAll();
	/* install forwarding strategy on the node */
	ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
	/* for a node - the router (node 2) */
	Ptr<ns3::ndn::L3Protocol> l3 = ns3::ndn::L3Protocol::getL3Protocol(nodes.Get(2));
	/* get the content store for the node */
	nfd::Cs &myContentStore = l3->getForwarder()->getCs();
	myContentStore.setLimit(20);
	/* -----------| set policy for Cs */
	unique_ptr<JPBPrintContent> policy = make_unique<JPBPrintContent>();
	policy->setLimit(20); /* should be same as Cs::setLimit */
	myContentStore.setPolicy(move(policy));
	/* consumer 1 */
	ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
	consumerHelper.SetPrefix("/prefix/c1");
	consumerHelper.SetAttribute("Frequency", StringValue("10"));
	consumerHelper.Install(nodes.Get(0));
	/* consumer 2 */
	ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerCbr");
	consumerHelper2.SetPrefix("/prefix/c2");
	consumerHelper2.SetAttribute("Frequency", StringValue("10"));
	consumerHelper2.Install(nodes.Get(1));
	/* producer */
	ndn::AppHelper producerHelper("ns3::ndn::Producer");
	producerHelper.SetPrefix("/prefix");
	producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
	producerHelper.Install(nodes.Get(3));
	/* run simulation */
	Simulator::Stop(Seconds(20.0));
	Simulator::Run();
	cout << "Simulation complete" << endl;
	Simulator::Destroy();
	return 0;
}
} // namespace ns3

int main(int argc, char *argv[])
{
	return ns3::main(argc, argv);
}


