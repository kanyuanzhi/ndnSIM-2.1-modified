#include "ns3/ndnSIM/NFD/daemon/table/cs-policy.hpp"
#include <iostream>
#include <string>
using namespace std;
const string POLICY_NAME = "JPBPolicy";
class JPBPolicy : public nfd::cs::Policy
{
  public:
	JPBPolicy() : nfd::cs::Policy(POLICY_NAME) {}

  protected:
	void doAfterInsert(nfd::cs::iterator it)
	{
		auto dataIt = it->getData();
		 cout << "Just inserted something (JPBPolicy)" << dataIt.getName() << endl;
	}
	// end doAfterInsert
	void doAfterRefresh(nfd::cs::iterator it) { cout << "Just refresh something (JPBPolicy)" << endl; }
	// end doAfterRefresh
	void doBeforeErase(nfd::cs::iterator it) { cout << "Ready to erase something (JPBPolicy)" << endl; }
	// end doBeforeErase
	void doBeforeUse(nfd::cs::iterator it) {}
	// end doBeforeUse
	void evictEntries() {}
	// end evictEntries
};
// end of JPBPolicyClass