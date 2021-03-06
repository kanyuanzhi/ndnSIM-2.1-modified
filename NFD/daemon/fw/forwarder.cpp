/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne
 * University, Washington University in St. Louis, Beijing Institute of
 * Technology, The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "forwarder.hpp"
#include "core/logger.hpp"
#include "core/random.hpp"
#include "face/null-face.hpp"
#include "strategy.hpp"

#include "utils/ndn-ns3-packet-tag.hpp"

#include <boost/random/uniform_int_distribution.hpp>
#include <sstream>
#include <string>

using namespace std;

namespace nfd {

NFD_LOG_INIT( "Forwarder" );

using fw::Strategy;

const Name Forwarder::LOCALHOST_NAME( "ndn:/localhost" );

Forwarder::Forwarder()
    : m_faceTable( *this )
    // , m_rand(CreateObject<ns3::UniformRandomVariable>())
    , m_fib( m_nameTree )
    , m_pit( m_nameTree )
    , m_measurements( m_nameTree )
    , m_strategyChoice( m_nameTree, fw::makeDefaultStrategy( *this ) )
    , m_csFace( make_shared<NullFace>( FaceUri( "contentstore://" ) ) ) {
  fw::installStrategies( *this );
  getFaceTable().addReserved( m_csFace, FACEID_CONTENT_STORE );
}

Forwarder::~Forwarder() {}

void Forwarder::onIncomingInterest( Face &inFace, const Interest &interest ) {
  // add by kan 20190324 20190330
  // Interest* interest = const_cast<Interest*>(&interest);
  if ( interest.getValidationFlag() == 1 ) {
    std::string PITListStr = interest.getPITList();
    PITListStr += std::to_string( inFace.getId() ) + " ";
    const_cast<Interest &>( interest ).setPITList( PITListStr );
  }

  // int node = ns3::Simulator::GetContext();
  // std::cout<<"node "<<node<<" get interest with PITList value
  // "<<interest.getPITList()<<std::endl; std::cout<<"node "<<node<<"
  // "<<inFace.getId()<<std::endl;
  // std::cout<<inFace.getId()<<std::endl;
  // end add

  // receive Interest
  NFD_LOG_DEBUG( "onIncomingInterest face=" << inFace.getId() << " interest="
                                            << interest.getName() );
  const_cast<Interest &>( interest ).setIncomingFaceId( inFace.getId() );

  // add by kan 20201231
  if ( interest.getLocationRegistration() == 1 ) {
    // 注册位置的兴趣包直接转发,不经过PIT表
    NFD_LOG_DEBUG( "getLocationRegistration" );
    shared_ptr<pit::Entry>       pitEntry = make_shared<pit::Entry>( interest );
    shared_ptr<name_tree::Entry> nameTreeEntry =
        m_pit.getNameTree().lookup( interest.getName() );
    // shared_ptr<pit::Entry> entry = make_shared<pit::Entry>(interest);
    nameTreeEntry->insertPitEntry( pitEntry );
    // NFD_LOG_DEBUG( pitEntry->getName() );
    // // FIB lookup
    shared_ptr<fib::Entry> fibEntry =
        m_fib.findLongestPrefixMatch( pitEntry->getName() );

    const fib::NextHopList &nexthops = fibEntry->getNextHops();
    // fib::NextHopList::const_iterator it       = std::find_if(
    //     nexthops.begin(), nexthops.end(),
    //     bind( &predicate_PitEntry_canForwardTo_NextHop, pitEntry, _1 ) );
    shared_ptr<Face> outFace = nexthops.begin()->getFace();
    outFace->sendInterest( interest );

    // dispatch to strategy
    // this->dispatchToStrategy(
    //     pitEntry, bind( &Strategy::afterReceiveInterest, _1, cref( inFace ),
    //                     cref( interest ), fibEntry, pitEntry ) );
    // shared_ptr<pit::Entry> pitEntry = m_pit.insert( interest ).first;

    // this->onContentStoreMiss( inFace, pitEntry, interest );
  } else {
    ++m_counters.getNInInterests();

    // /localhost scope control
    bool isViolatingLocalhost =
        !inFace.isLocal() && LOCALHOST_NAME.isPrefixOf( interest.getName() );
    if ( isViolatingLocalhost ) {
      NFD_LOG_DEBUG( "onIncomingInterest face="
                     << inFace.getId() << " interest=" << interest.getName()
                     << " violates /localhost" );
      // (drop)
      return;
    }

    // PIT insert
    shared_ptr<pit::Entry> pitEntry = m_pit.insert( interest ).first;

    // detect duplicate Nonce
    int  dnw = pitEntry->findNonce( interest.getNonce(), inFace );
    bool hasDuplicateNonce =
        ( dnw != pit::DUPLICATE_NONCE_NONE ) ||
        m_deadNonceList.has( interest.getName(), interest.getNonce() );
    if ( hasDuplicateNonce ) {
      // goto Interest loop pipeline
      this->onInterestLoop( inFace, interest, pitEntry );
      return;
    }

    // cancel unsatisfy & straggler timer
    this->cancelUnsatisfyAndStragglerTimer( pitEntry );

    // is pending?
    // isPending为True时，表示有两条及以上记录，对于普通请求，cs表中肯定无记录，对于有效性请求，可能有记录
    const pit::InRecordCollection &inRecords = pitEntry->getInRecords();
    bool isPending = inRecords.begin() != inRecords.end();
    if ( !isPending ) {
      // 由于实验使用oldContentStrore，此处并未考虑到m_csFromNdnSim==nullptr的情况
      if ( m_csFromNdnSim == nullptr ) {
        m_cs.find( interest,
                   bind( &Forwarder::onContentStoreHit, this, ref( inFace ),
                         pitEntry, _1, _2 ),
                   bind( &Forwarder::onContentStoreMiss, this, ref( inFace ),
                         pitEntry, _1 ) );
      } else {
        shared_ptr<Data> match =
            m_csFromNdnSim->Lookup( interest.shared_from_this() );
        if ( match != nullptr ) {
          // add by kan 20190409
          // 判断过期字段是否为1，若为1，表示该数据包是服务器在删除PITListStore中的记录时发出的，
          // 此时该数据包有可能不是最新，需要重新向服务器发起请求。
          // cout << "exp: " <<match->getExpiration() <<endl;
          // if ( match->getExpiration() == 1 ) {
          //   cout << "××××××××××××××××××××××××××××××××××××××××××××××××××" <<
          //   endl;
          //   this->onContentStoreMiss( inFace, pitEntry, interest );
          // } else
          this->onContentStoreHit( inFace, pitEntry, interest, *match );
          // end add
        } else {
          this->onContentStoreMiss( inFace, pitEntry, interest );
        }
      }
    } else {
      // add by kan 20190410
      // 当PIT表中中有聚合记录时，对于有效性请求，需要查询CS中是否有记录
      if ( interest.getValidationFlag() == 1 ) {
        shared_ptr<Data> match =
            m_csFromNdnSim->Lookup( interest.shared_from_this() );
        if ( match != nullptr ) {
          // 判断过期字段是否为1，若为1，表示该数据包是服务器在删除PITListStore中的记录时发出的，
          // 此时该数据包有可能不是最新，需要重新向服务器发起请求。
          // if ( match->getExpiration() == 1 )
          //   this->onContentStoreMiss( inFace, pitEntry, interest );
          // else
          NFD_LOG_DEBUG( "match != nullptr" );
          this->onContentStoreHit( inFace, pitEntry, interest, *match );

        } else {
          this->onContentStoreMiss( inFace, pitEntry, interest );
        }
        // end add
      } else {
        this->onContentStoreMiss( inFace, pitEntry, interest );
      }
    }
  }
}

void Forwarder::onContentStoreMiss( const Face &           inFace,
                                    shared_ptr<pit::Entry> pitEntry,
                                    const Interest &       interest ) {
  if ( interest.getLocationRegistration() == 1 ) {
    NFD_LOG_DEBUG( "onLocationRegistration interest=" << interest.getName() );
  } else {
    NFD_LOG_DEBUG( "onContentStoreMiss interest=" << interest.getName() );
  }

  shared_ptr<Face> face = const_pointer_cast<Face>( inFace.shared_from_this() );
  // insert InRecord
  pitEntry->insertOrUpdateInRecord( face, interest );
  if ( interest.getLocationRegistration() == 0 ) {
    // set PIT unsatisfy timer
    this->setUnsatisfyTimer( pitEntry );
  }

  // FIB lookup
  shared_ptr<fib::Entry> fibEntry = m_fib.findLongestPrefixMatch( *pitEntry );

  // dispatch to strategy
  this->dispatchToStrategy( pitEntry, bind( &Strategy::afterReceiveInterest, _1,
                                            cref( inFace ), cref( interest ),
                                            fibEntry, pitEntry ) );
  if ( interest.getLocationRegistration() == 1 ) {
    NFD_LOG_DEBUG( m_pit.size() );
    NFD_LOG_DEBUG( "m_pit.erase1" );

    m_pit.erase( pitEntry );
    NFD_LOG_DEBUG( m_pit.size() );
    NFD_LOG_DEBUG( "m_pit.erase2" );
  }
}

void Forwarder::onContentStoreHit( const Face &           inFace,
                                   shared_ptr<pit::Entry> pitEntry,
                                   const Interest &       interest,
                                   const Data &           data ) {
  NFD_LOG_DEBUG( "onContentStoreHit interest=" << interest.getName() );
  // cout<<"onContentStoreHit interest=" << interest.getName() <<endl;
  beforeSatisfyInterest( *pitEntry, *m_csFace, data );
  this->dispatchToStrategy( pitEntry,
                            bind( &Strategy::beforeSatisfyInterest, _1,
                                  pitEntry, cref( *m_csFace ), cref( data ) ) );

  const_pointer_cast<Data>( data.shared_from_this() )
      ->setIncomingFaceId( FACEID_CONTENT_STORE );
  // XXX should we lookup PIT for other Interests that also match csMatch?

  // set PIT straggler timer
  this->setStragglerTimer( pitEntry, true, data.getFreshnessPeriod() );

  // goto outgoing Data pipeline
  this->onOutgoingData(
      data, *const_pointer_cast<Face>( inFace.shared_from_this() ) );
}

void Forwarder::onInterestLoop( Face &inFace, const Interest &interest,
                                shared_ptr<pit::Entry> pitEntry ) {
  NFD_LOG_DEBUG( "onInterestLoop face=" << inFace.getId()
                                        << " interest=" << interest.getName() );

  // (drop)
}

/** \brief compare two InRecords for picking outgoing Interest
 *  \return true if b is preferred over a
 *
 *  This function should be passed to std::max_element over InRecordCollection.
 *  The outgoing Interest picked is the last incoming Interest
 *  that does not come from outFace.
 *  If all InRecords come from outFace, it's fine to pick that. This happens
 * when there's only one InRecord that comes from outFace. The legit use is for
 *  vehicular network; otherwise, strategy shouldn't send to the sole inFace.
 */
static inline bool compare_pickInterest( const pit::InRecord &a,
                                         const pit::InRecord &b,
                                         const Face *         outFace ) {
  bool isOutFaceA = a.getFace().get() == outFace;
  bool isOutFaceB = b.getFace().get() == outFace;

  if ( !isOutFaceA && isOutFaceB ) {
    return false;
  }
  if ( isOutFaceA && !isOutFaceB ) {
    return true;
  }

  return a.getLastRenewed() > b.getLastRenewed();
}

void Forwarder::onOutgoingInterest( shared_ptr<pit::Entry> pitEntry,
                                    Face &outFace, bool wantNewNonce ) {
  if ( outFace.getId() == INVALID_FACEID ) {
    NFD_LOG_WARN(
        "onOutgoingInterest face=invalid interest=" << pitEntry->getName() );
    return;
  }
  NFD_LOG_DEBUG( "onOutgoingInterest face=" << outFace.getId() << " interest="
                                            << pitEntry->getName() );

  // scope control
  if ( pitEntry->violatesScope( outFace ) ) {
    NFD_LOG_DEBUG( "onOutgoingInterest face=" << outFace.getId() << " interest="
                                              << pitEntry->getName()
                                              << " violates scope" );
    return;
  }

  // pick Interest
  const pit::InRecordCollection &inRecords = pitEntry->getInRecords();
  pit::InRecordCollection::const_iterator pickedInRecord =
      std::max_element( inRecords.begin(), inRecords.end(),
                        bind( &compare_pickInterest, _1, _2, &outFace ) );
  BOOST_ASSERT( pickedInRecord != inRecords.end() );
  shared_ptr<Interest> interest = const_pointer_cast<Interest>(
      pickedInRecord->getInterest().shared_from_this() );

  if ( wantNewNonce ) {
    interest = make_shared<Interest>( *interest );
    static boost::random::uniform_int_distribution<uint32_t> dist;
    interest->setNonce( dist( getGlobalRng() ) );
  }

  // insert OutRecord
  pitEntry->insertOrUpdateOutRecord( outFace.shared_from_this(), *interest );

  // send Interest
  outFace.sendInterest( *interest );
  ++m_counters.getNOutInterests();
}

void Forwarder::onInterestReject( shared_ptr<pit::Entry> pitEntry ) {
  if ( pitEntry->hasUnexpiredOutRecords() ) {
    NFD_LOG_ERROR( "onInterestReject interest="
                   << pitEntry->getName()
                   << " cannot reject forwarded Interest" );
    return;
  }
  NFD_LOG_DEBUG( "onInterestReject interest=" << pitEntry->getName() );

  // cancel unsatisfy & straggler timer
  this->cancelUnsatisfyAndStragglerTimer( pitEntry );

  // set PIT straggler timer
  this->setStragglerTimer( pitEntry, false );
}

void Forwarder::onInterestUnsatisfied( shared_ptr<pit::Entry> pitEntry ) {
  NFD_LOG_DEBUG( "onInterestUnsatisfied interest=" << pitEntry->getName() );

  // invoke PIT unsatisfied callback
  beforeExpirePendingInterest( *pitEntry );
  this->dispatchToStrategy(
      pitEntry, bind( &Strategy::beforeExpirePendingInterest, _1, pitEntry ) );

  // goto Interest Finalize pipeline
  this->onInterestFinalize( pitEntry, false );
}

void Forwarder::onInterestFinalize(
    shared_ptr<pit::Entry> pitEntry, bool isSatisfied,
    const time::milliseconds &dataFreshnessPeriod ) {
  NFD_LOG_DEBUG( "onInterestFinalize interest="
                 << pitEntry->getName()
                 << ( isSatisfied ? " satisfied" : " unsatisfied" ) );

  // Dead Nonce List insert if necessary
  this->insertDeadNonceList( *pitEntry, isSatisfied, dataFreshnessPeriod, 0 );

  // PIT delete
  this->cancelUnsatisfyAndStragglerTimer( pitEntry );
  m_pit.erase( pitEntry );
}

void Forwarder::onIncomingData( Face &inFace, const Data &data ) {
  // add by kan 20190401 20190402
  ++m_counters.getNInDatas();
  int node = ns3::Simulator::GetContext();
  // std::cout << "node " << node << " get data with value "
  //           << data.getPITListBack() << " " << data.getValidationDataFlag()
  //           << " " << data.getExpiration() << std::endl;
  // cout << "node " << node << " size is " <<
  // m_csFromNdnSim->Begin()->GetName()
  //      << ", limit is " << endl;
  int ValidationFlag        = data.getValidationDataFlag();    // 1或0
  int ValidationPublishment = data.getValidationPublishment(); // 1或0
  int Expiration            = data.getExpiration();
  int Eligibility           = data.getEligibility();

  // cout << node << " : " << data.getName() << " : " << ValidationPublishment
  //      << endl;
  // 读取PITListBack中的当前节点入端口号
  if ( ValidationFlag == 1 && ValidationPublishment == 1 ) {
    // 有有效性要求，且是服务器主动发布的数据
    std::vector<std::string> res;
    std::string              PITList = data.getPITListBack();
    std::string              result;
    std::stringstream        input( PITList );
    while ( input >> result ) {
      res.push_back( result );
    }

    // cout << node << " : " << data.getName() << " : "<<PITList << " : " <<
    // res.size()<< endl;
    //      << ValidationPublishment << endl;
    int         port            = std::stoi( res[ res.size() - 1 ] );
    std::string PITListBackTemp = "";
    for ( unsigned int i = 0; i < res.size() - 1; i++ ) {
      PITListBackTemp += res[ i ] + " ";
    }

    const_cast<Data &>( data ).setPITListBack( PITListBackTemp );
    const_cast<Data &>( data ).setIncomingFaceId( inFace.getId() );
    shared_ptr<Face> outFace = Forwarder::getFace( port );
    this->onOutgoingData( data, *outFace );

    // 服务器主动发布的数据存储在沿途节点前，需要将ValidationPublishment置0
    const_cast<Data &>( data ).setValidationPublishment( 0 );

    shared_ptr<Data> dataCopyWithoutPacket = make_shared<Data>( data );
    dataCopyWithoutPacket->removeTag<ns3::ndn::Ns3PacketTag>();
    if ( Expiration == 1 ) {
      // Expiration字段为1，则是过期内容，将cs表中该内容删除
      // cout << "before: " <<m_csFromNdnSim->GetSize() <<endl;
      m_csFromNdnSim->Erase( dataCopyWithoutPacket );
      // NFD_LOG_DEBUG("Expiration");
      // cout << "Node-" << node << " : "<< data.getName() << " : " <<
      // data.getPITListBack() <<endl; cout << "after: " <<
      // m_csFromNdnSim->GetSize() <<endl;
    } else {
      // CS insert
      // NFD_LOG_DEBUG("Pubulishment");
      if ( m_csFromNdnSim == nullptr ) {
        m_cs.insert( *dataCopyWithoutPacket );
      } else {
        m_csFromNdnSim->Erase( dataCopyWithoutPacket );
        m_csFromNdnSim->Add( dataCopyWithoutPacket );
      }
    }

    // cout<< to_string(port)<<endl;
    // 有有效性要求的内容仅在边缘节点存储
    // 20190412修改：允许全节点缓存
    // if ( res.size() == 1 ) {
    //   // 此处是边缘节点
    //   shared_ptr<Data> dataCopyWithoutPacket = make_shared<Data>( data );
    //   dataCopyWithoutPacket->removeTag<ns3::ndn::Ns3PacketTag>();
    //   // CS insert
    //   if ( m_csFromNdnSim == nullptr ) {
    //     m_cs.insert( *dataCopyWithoutPacket );
    //   } else {
    //     m_csFromNdnSim->Add( dataCopyWithoutPacket );
    //   }
    //   // cout << PITList << endl;
    //   // shared_ptr<Face> outFace = getFace( port );
    //   // this->onOutgoingData( data, *outFace );
    //   // return;
    // }

    // 根据PITListBack记录的端口号转发有有效性要求的data

    // outFace->sendData( data );
    // ++m_counters.getNOutDatas();

  }
  // end add

  // add by kan 20191231
  else if ( ValidationFlag == 1 && Eligibility == 1 ) {
    // receive Data
    NFD_LOG_DEBUG( "onIncomingData face=" << inFace.getId()
                                          << " data=" << data.getName() );
    const_cast<Data &>( data ).setIncomingFaceId( inFace.getId() );
    // ++m_counters.getNInDatas();

    // /localhost scope control
    bool isViolatingLocalhost =
        !inFace.isLocal() && LOCALHOST_NAME.isPrefixOf( data.getName() );
    if ( isViolatingLocalhost ) {
      NFD_LOG_DEBUG( "onIncomingData face=" << inFace.getId()
                                            << " data=" << data.getName()
                                            << " violates /localhost" );
      // (drop)
      return;
    }
    // PIT match
    pit::DataMatchResult pitMatches = m_pit.findAllDataMatches( data );
    if ( pitMatches.begin() == pitMatches.end() ) {
      // goto Data unsolicited pipeline
      this->onDataUnsolicited( inFace, data );
      return;
    }

    // Remove Ptr<Packet> from the Data before inserting into cache, serving two
    // purposes
    // - reduce amount of memory used by cached entries
    // - remove all tags that (e.g., hop count tag) that could have been
    // associated with Ptr<Packet>
    //
    // Copying of Data is relatively cheap operation, as it copies (mostly) a
    // collection of Blocks pointing to the same underlying memory buffer.
    shared_ptr<Data> dataCopyWithoutPacket = make_shared<Data>( data );
    dataCopyWithoutPacket->removeTag<ns3::ndn::Ns3PacketTag>();
    // CS insert
    if ( m_csFromNdnSim == nullptr ) {
      m_cs.insert( *dataCopyWithoutPacket );
    } else {
      shared_ptr<Interest> interest = make_shared<Interest>();
      // ns3::Ptr<ns3::UniformRandomVariable> m_rand =
      // CreateObject<ns3::UniformRandomVariable>();
      // interest->setNonce(m_rand->GetValue( 0,
      // std::numeric_limits<uint32_t>::max() ) );
      interest->setName( data.getName() );
      shared_ptr<Data> match = m_csFromNdnSim->Lookup( interest );

      if ( match != nullptr ) {
        // 先删除旧内容再缓存新内容
        m_csFromNdnSim->Erase( dataCopyWithoutPacket );
        m_csFromNdnSim->Add( dataCopyWithoutPacket );
      } else {
        // add by kan 20191231
        interest->setPITList( "" );
        interest->setValidationFlag( 1 );
        // end add
        interest->setLocationRegistration( 1 );
        NFD_LOG_DEBUG( "setLocationRegistration" );
        static boost::random::uniform_int_distribution<uint32_t> dist;

        interest->setNonce( dist( getGlobalRng() ) );
        inFace.sendInterest( *interest );
        m_csFromNdnSim->Add( dataCopyWithoutPacket );
      }

      // cout << m_csFromNdnSim->GetSize() << endl;
    }

    std::set<shared_ptr<Face>> pendingDownstreams;
    // foreach PitEntry
    for ( const shared_ptr<pit::Entry> &pitEntry : pitMatches ) {
      NFD_LOG_DEBUG( "onIncomingData matching=" << pitEntry->getName() );

      // cancel unsatisfy & straggler timer
      this->cancelUnsatisfyAndStragglerTimer( pitEntry );

      // remember pending downstreams
      const pit::InRecordCollection &inRecords = pitEntry->getInRecords();
      for ( pit::InRecordCollection::const_iterator it = inRecords.begin();
            it != inRecords.end(); ++it ) {
        if ( it->getExpiry() > time::steady_clock::now() ) {
          pendingDownstreams.insert( it->getFace() );
        }
      }

      // invoke PIT satisfy callback
      beforeSatisfyInterest( *pitEntry, inFace, data );
      this->dispatchToStrategy(
          pitEntry, bind( &Strategy::beforeSatisfyInterest, _1, pitEntry,
                          cref( inFace ), cref( data ) ) );

      // Dead Nonce List insert if necessary (for OutRecord of inFace)
      this->insertDeadNonceList( *pitEntry, true, data.getFreshnessPeriod(),
                                 &inFace );

      // mark PIT satisfied
      pitEntry->deleteInRecords();
      pitEntry->deleteOutRecord( inFace );

      // set PIT straggler timer
      this->setStragglerTimer( pitEntry, true, data.getFreshnessPeriod() );
    }
    // foreach pending downstream
    for ( std::set<shared_ptr<Face>>::iterator it = pendingDownstreams.begin();
          it != pendingDownstreams.end(); ++it ) {
      shared_ptr<Face> pendingDownstream = *it;
      if ( pendingDownstream.get() == &inFace ) {
        continue;
      }
      // goto outgoing Data pipeline
      this->onOutgoingData( data, *pendingDownstream );
    }
  }
  // end add
  else {
    // 无有效性要求，或是有效性要求但属于正常请求响应返回的的数据
    // receive Data
    NFD_LOG_DEBUG( "onIncomingData face=" << inFace.getId()
                                          << " data=" << data.getName() );
    const_cast<Data &>( data ).setIncomingFaceId( inFace.getId() );
    // ++m_counters.getNInDatas();

    // /localhost scope control
    bool isViolatingLocalhost =
        !inFace.isLocal() && LOCALHOST_NAME.isPrefixOf( data.getName() );
    if ( isViolatingLocalhost ) {
      NFD_LOG_DEBUG( "onIncomingData face=" << inFace.getId()
                                            << " data=" << data.getName()
                                            << " violates /localhost" );
      // (drop)
      return;
    }
    // PIT match
    pit::DataMatchResult pitMatches = m_pit.findAllDataMatches( data );
    if ( pitMatches.begin() == pitMatches.end() ) {
      // goto Data unsolicited pipeline
      this->onDataUnsolicited( inFace, data );
      return;
    }

    // Remove Ptr<Packet> from the Data before inserting into cache, serving two
    // purposes
    // - reduce amount of memory used by cached entries
    // - remove all tags that (e.g., hop count tag) that could have been
    // associated with Ptr<Packet>
    //
    // Copying of Data is relatively cheap operation, as it copies (mostly) a
    // collection of Blocks pointing to the same underlying memory buffer
    if ( ValidationFlag == 0 ) {
      shared_ptr<Data> dataCopyWithoutPacket = make_shared<Data>( data );
      dataCopyWithoutPacket->removeTag<ns3::ndn::Ns3PacketTag>();
      // CS insert
      if ( m_csFromNdnSim == nullptr ) {
        m_cs.insert( *dataCopyWithoutPacket );
      } else {
        // 先删除旧内容再缓存新内容
        m_csFromNdnSim->Erase( dataCopyWithoutPacket );
        m_csFromNdnSim->Add( dataCopyWithoutPacket );
        // cout << m_csFromNdnSim->GetSize() << endl;
      }
    }

    std::set<shared_ptr<Face>> pendingDownstreams;
    // foreach PitEntry
    for ( const shared_ptr<pit::Entry> &pitEntry : pitMatches ) {
      NFD_LOG_DEBUG( "onIncomingData matching=" << pitEntry->getName() );

      // cancel unsatisfy & straggler timer
      this->cancelUnsatisfyAndStragglerTimer( pitEntry );

      // remember pending downstreams
      const pit::InRecordCollection &inRecords = pitEntry->getInRecords();
      for ( pit::InRecordCollection::const_iterator it = inRecords.begin();
            it != inRecords.end(); ++it ) {
        if ( it->getExpiry() > time::steady_clock::now() ) {
          pendingDownstreams.insert( it->getFace() );
        }
      }

      // invoke PIT satisfy callback
      beforeSatisfyInterest( *pitEntry, inFace, data );
      this->dispatchToStrategy(
          pitEntry, bind( &Strategy::beforeSatisfyInterest, _1, pitEntry,
                          cref( inFace ), cref( data ) ) );

      // Dead Nonce List insert if necessary (for OutRecord of inFace)
      this->insertDeadNonceList( *pitEntry, true, data.getFreshnessPeriod(),
                                 &inFace );

      // mark PIT satisfied
      pitEntry->deleteInRecords();
      pitEntry->deleteOutRecord( inFace );

      // set PIT straggler timer
      this->setStragglerTimer( pitEntry, true, data.getFreshnessPeriod() );
    }
    // foreach pending downstream
    for ( std::set<shared_ptr<Face>>::iterator it = pendingDownstreams.begin();
          it != pendingDownstreams.end(); ++it ) {
      shared_ptr<Face> pendingDownstream = *it;
      if ( pendingDownstream.get() == &inFace ) {
        continue;
      }
      // goto outgoing Data pipeline
      this->onOutgoingData( data, *pendingDownstream );
    }
  }
}

void Forwarder::onDataUnsolicited( Face &inFace, const Data &data ) {
  // accept to cache?
  bool acceptToCache = inFace.isLocal();
  if ( acceptToCache ) {
    // CS insert
    if ( m_csFromNdnSim == nullptr )
      m_cs.insert( data, true );
    else
      m_csFromNdnSim->Add( data.shared_from_this() );
  }

  NFD_LOG_DEBUG( "onDataUnsolicited face="
                 << inFace.getId() << " data=" << data.getName()
                 << ( acceptToCache ? " cached" : " not cached" ) << " "
                 << data.getValidationDataFlag() << " "
                 << data.getValidationPublishment() << " "
                 << data.getExpiration() << " " << data.getEligibility() );
}

void Forwarder::onOutgoingData( const Data &data, Face &outFace ) {
  if ( outFace.getId() == INVALID_FACEID ) {
    NFD_LOG_WARN( "onOutgoingData face=invalid data="
                  << data.getName() << data.getValidationDataFlag()
                  << data.getValidationPublishment() << data.getExpiration()
                  << data.getEligibility() );
    return;
  }
  NFD_LOG_DEBUG( "onOutgoingData face=" << outFace.getId()
                                        << " data=" << data.getName() << " "
                                        << data.getValidationDataFlag() << " "
                                        << data.getValidationPublishment()
                                        << " " << data.getExpiration() << " "
                                        << data.getEligibility() << " " );

  // /localhost scope control
  bool isViolatingLocalhost =
      !outFace.isLocal() && LOCALHOST_NAME.isPrefixOf( data.getName() );
  if ( isViolatingLocalhost ) {
    NFD_LOG_DEBUG( "onOutgoingData face=" << outFace.getId()
                                          << " data=" << data.getName()
                                          << " violates /localhost" );
    // (drop)
    return;
  }

  // TODO traffic manager

  // send Data
  outFace.sendData( data );
  ++m_counters.getNOutDatas();
}

static inline bool compare_InRecord_expiry( const pit::InRecord &a,
                                            const pit::InRecord &b ) {
  return a.getExpiry() < b.getExpiry();
}

void Forwarder::setUnsatisfyTimer( shared_ptr<pit::Entry> pitEntry ) {
  const pit::InRecordCollection &         inRecords = pitEntry->getInRecords();
  pit::InRecordCollection::const_iterator lastExpiring = std::max_element(
      inRecords.begin(), inRecords.end(), &compare_InRecord_expiry );

  time::steady_clock::TimePoint lastExpiry = lastExpiring->getExpiry();
  time::nanoseconds lastExpiryFromNow = lastExpiry - time::steady_clock::now();
  if ( lastExpiryFromNow <= time::seconds( 0 ) ) {
    // TODO all InRecords are already expired; will this happen?
  }

  scheduler::cancel( pitEntry->m_unsatisfyTimer );
  pitEntry->m_unsatisfyTimer = scheduler::schedule(
      lastExpiryFromNow,
      bind( &Forwarder::onInterestUnsatisfied, this, pitEntry ) );
}

void Forwarder::setStragglerTimer(
    shared_ptr<pit::Entry> pitEntry, bool isSatisfied,
    const time::milliseconds &dataFreshnessPeriod ) {
  time::nanoseconds stragglerTime = time::milliseconds( 100 );

  scheduler::cancel( pitEntry->m_stragglerTimer );
  pitEntry->m_stragglerTimer = scheduler::schedule(
      stragglerTime, bind( &Forwarder::onInterestFinalize, this, pitEntry,
                           isSatisfied, dataFreshnessPeriod ) );
}

void Forwarder::cancelUnsatisfyAndStragglerTimer(
    shared_ptr<pit::Entry> pitEntry ) {
  scheduler::cancel( pitEntry->m_unsatisfyTimer );
  scheduler::cancel( pitEntry->m_stragglerTimer );
}

static inline void insertNonceToDnl( DeadNonceList &       dnl,
                                     const pit::Entry &    pitEntry,
                                     const pit::OutRecord &outRecord ) {
  dnl.add( pitEntry.getName(), outRecord.getLastNonce() );
}

void Forwarder::insertDeadNonceList(
    pit::Entry &pitEntry, bool isSatisfied,
    const time::milliseconds &dataFreshnessPeriod, Face *upstream ) {
  // need Dead Nonce List insert?
  bool needDnl = false;
  if ( isSatisfied ) {
    bool hasFreshnessPeriod = dataFreshnessPeriod >= time::milliseconds::zero();
    // Data never becomes stale if it doesn't have FreshnessPeriod field
    needDnl = static_cast<bool>( pitEntry.getInterest().getMustBeFresh() ) &&
              ( hasFreshnessPeriod &&
                dataFreshnessPeriod < m_deadNonceList.getLifetime() );
  } else {
    needDnl = true;
  }

  if ( !needDnl ) {
    return;
  }

  // Dead Nonce List insert
  if ( upstream == 0 ) {
    // insert all outgoing Nonces
    const pit::OutRecordCollection &outRecords = pitEntry.getOutRecords();
    std::for_each( outRecords.begin(), outRecords.end(),
                   bind( &insertNonceToDnl, ref( m_deadNonceList ),
                         cref( pitEntry ), _1 ) );
  } else {
    // insert outgoing Nonce of a specific face
    pit::OutRecordCollection::const_iterator outRecord =
        pitEntry.getOutRecord( *upstream );
    if ( outRecord != pitEntry.getOutRecords().end() ) {
      m_deadNonceList.add( pitEntry.getName(), outRecord->getLastNonce() );
    }
  }
}

} // namespace nfd
