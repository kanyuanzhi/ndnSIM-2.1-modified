#include <iostream>
//core module and required helpers for example
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
//required for use of CS (from NFD)
#include "ns3/ndnSIM/NFD/daemon/table/cs.hpp"
#include "ns3/ndnSIM/NFD/daemon/table/cs-policy-priority-fifo.hpp"
#include "ns3/ndnSIM/NFD/core/logger.hpp"
#include "ns3/ndnSIM/NFD/core/algorithm.hpp"
#include "ns3/ndnSIM/ndn-cxx/data.hpp"
#include "umd/JPBPolicy.hpp"
#include <string>
#include <array>
using namespace std;

template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

namespace ns3 {
	//----------------------------------------------------//
	//--------------| function prototypes |---------------//
	//----------------------------------------------------//
	void getArrayOfData(array<shared_ptr<ndn::Data>, 5>& arrData);
	void storeDataInCS(array<shared_ptr<ndn::Data>, 5> arrData, nfd::Cs& theCs);
	int
	main(int argc, char* argv[]) {
		//read optional command line params
		CommandLine cmd;
		cmd.Parse(argc, argv);
		//Create a single node
		//through the node container
		NodeContainer nodes;
		nodes.Create(1);
		//only one node for this example
		//install the NDN stack on the node
		ndn::StackHelper ndnHelper;
		ndnHelper.SetDefaultRoutes(true);
		ndnHelper.InstallAll();
		//install forwarding strategy on the node
		ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
		//Get the L3Protocol so that you can
		//obtain the Content Store using getCs
		Ptr<ns3::ndn::L3Protocol> l3 =
		ns3::ndn::L3Protocol::getL3Protocol(nodes.Get(0));
		//get the content store for the node
		nfd::Cs& myContentStore = l3->getForwarder()->getCs();
		//set the limit to 100 items
		myContentStore.setLimit(100);
		//create an array to hold data items
		array<shared_ptr<ndn::Data>, 5> arrData;		//create an array to hold the data packets
 
		getArrayOfData(arrData);//call function to populate the array
		
		//print test - print the data from the data array
		for (int i = 0; i < 5; i++) {
			//create an array to hold
			cout<<(arrData[i])->getName()<<endl;
		}
        
		//-----------| set policy for Cs |---------------//
        unique_ptr<JPBPolicy> policy = make_unique<JPBPolicy>();
        myContentStore.setPolicy(move(policy));

        cout<<"After move policy"<<endl;
        storeDataInCS(arrData, myContentStore);
        cout<<"After storeDataInCS"<<endl;

		//print from content store
		//to make sure the items live there
		cout<<"About to print the Content Store data items"<<endl;
		//use iterators to move through the content store
		for (auto it = myContentStore.begin(); it != myContentStore.end(); ++it) {
			cout<<it->getName()<<endl;
			//it represents a shared_ptr
			//to a single Data item
		}
		cout<<"Hello world."<<endl;
		return 0;
	}
	//end ns3::main function
	//-----------------------------------------------------------//
	//---------------| getArrayOfData |--------------------------//
	//-----------------------------------------------------------//
	void
	getArrayOfData(array<shared_ptr<ndn::Data>, 5>& myArrayOfData) {
		for (int i = 0; i < 5; i++) {
			shared_ptr<ndn::Data> tempD = make_shared<ndn::Data>();
			string testStr = "Test" + to_string(i);
			tempD->setName(ndn::Name(testStr));
			myArrayOfData[i] = tempD;
		}
		//end for
	}
	//end getArrayOfData
	//-----------------------------------------------------------//
	//----------------| storeDataInCS |--------------------------//
	//-----------------------------------------------------------//
	void
	storeDataInCS(array<shared_ptr<ndn::Data>, 5> arr, nfd::Cs& theCs) {
		for (int i = 0; i < 5; i++) {
			theCs.insert(*arr[i]);
		}
	}
	//end storeDataInCS
}
//end ns3 namespace for this file
//-----------------------------------------------------------//
//-------------| main function for program |-----------------//
//-----------------------------------------------------------//
int
main(int argc, char* argv[]) {
	return ns3::main(argc, argv);
}