/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <array>
#include <string>
#include <thread>
#include <vector>

#include "common/Constants.h"
#include "libDirectoryService/DirectoryService.h"
#include "libMediator/Mediator.h"
#include "libPOW/pow.h"
#include "libTestUtils/TestUtils.h"

#define BOOST_TEST_MODULE lookupnodetxblocktest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(test_por)

BOOST_AUTO_TEST_CASE(testTxBlockStoring) {
  INIT_STDOUT_LOGGER();

  LOG_MARKER();

  PairOfKey myKeypair = Schnorr::GetInstance().GenKeyPair();
  Peer myPeer = TestUtils::GenerateRandomPeer();

  Mediator mediator(myKeypair, myPeer);

  Synchronizer synchronizer;
  synchronizer.InitializeGenesisBlocks(mediator.m_dsBlockChain,
                                       mediator.m_txBlockChain);

  DirectoryService ds(mediator);
  const uint32_t EXTRA_NODE_NUM = 20;

  for (uint32_t i = 0; i < MAX_SHARD_NODE_NUM + EXTRA_NODE_NUM; ++i) {
    auto blockNumber =
        mediator.m_dsBlockChain.GetLastBlock().GetHeader().GetBlockNum();
    auto difficultyLevel = POW_DIFFICULTY;
    auto submitterPeer = TestUtils::GenerateRandomPeer();
    PairOfKey keypair = Schnorr::GetInstance().GenKeyPair();
    auto submitterKey = keypair.second;

    auto headerHash =
        POW::GenHeaderHash(mediator.m_dsBlockRand, mediator.m_txBlockRand,
                           submitterPeer.GetIpAddress(), submitterKey, 0, 0);

    auto winning_result = POW::GetInstance().PoWMine(
        blockNumber, difficultyLevel, keypair, headerHash, FULL_DATASET_MINE,
        std::time(0), POW_WINDOW_IN_SECONDS);

    DSPowSolution powSoln(blockNumber, difficultyLevel, submitterPeer,
                          submitterKey, winning_result.winning_nonce,
                          winning_result.result, winning_result.mix_hash, 0, 0,
                          Signature());
    ds.ProcessPoWSubmissionFromPacket(powSoln);
  }
}

BOOST_AUTO_TEST_SUITE_END()
