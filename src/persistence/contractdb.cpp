// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2017-2019 The WaykiChain Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "contractdb.h"

#include "entities/account.h"
#include "entities/id.h"
#include "entities/key.h"
#include "commons/uint256.h"
#include "commons/util.h"
#include "vm/luavm/luavmrunenv.h"

#include <stdint.h>

using namespace std;

/************************ contract account ******************************/
bool CContractDBCache::GetContractAccount(const CRegID &contractRegId, const string &accountKey,
                                          CAppUserAccount &appAccOut) {
    auto key = std::make_pair(contractRegId.ToRawString(), accountKey);
    return contractAccountCache.GetData(key, appAccOut);
}

bool CContractDBCache::SetContractAccount(const CRegID &contractRegId, const CAppUserAccount &appAccIn) {
    if (appAccIn.IsEmpty()) {
        return false;
    }
    auto key = std::make_pair(contractRegId.ToRawString(), appAccIn.GetAccUserId());
    return contractAccountCache.SetData(key, appAccIn);
}

/************************ contract in cache ******************************/
bool CContractDBCache::GetContract(const CRegID &contractRegId, CUniversalContract &contract) {
    return contractCache.GetData(contractRegId.ToRawString(), contract);
}

bool CContractDBCache::GetContracts(map<string, CUniversalContract> &contracts) {
    return contractCache.GetAllElements(contracts);
}

bool CContractDBCache::SaveContract(const CRegID &contractRegId, const CUniversalContract &contract) {
    return contractCache.SetData(contractRegId.ToRawString(), contract);
}

bool CContractDBCache::HaveContract(const CRegID &contractRegId) {
    return contractCache.HaveData(contractRegId.ToRawString());
}

bool CContractDBCache::EraseContract(const CRegID &contractRegId) {
    return contractCache.EraseData(contractRegId.ToRawString());
}

/************************ contract data ******************************/
bool CContractDBCache::GetContractData(const CRegID &contractRegId, const string &contractKey, string &contractData) {
    auto key = std::make_pair(contractRegId.ToRawString(), contractKey);
    return contractDataCache.GetData(key, contractData);
}

bool CContractDBCache::SetContractData(const CRegID &contractRegId, const string &contractKey,
                                       const string &contractData) {
    auto key = std::make_pair(contractRegId.ToRawString(), contractKey);
    return contractDataCache.SetData(key, contractData);
}

bool CContractDBCache::HaveContractData(const CRegID &contractRegId, const string &contractKey) {
    auto key = std::make_pair(contractRegId.ToRawString(), contractKey);
    return contractDataCache.HaveData(key);
}

bool CContractDBCache::EraseContractData(const CRegID &contractRegId, const string &contractKey) {
    auto key = std::make_pair(contractRegId.ToRawString(), contractKey);
    return contractDataCache.EraseData(key);
}

bool CContractDBCache::Flush() {
    contractCache.Flush();
    txDiskPosCache.Flush();
    contractRelatedKidCache.Flush();
    contractDataCache.Flush();
    contractAccountCache.Flush();

    return true;
}

uint32_t CContractDBCache::GetCacheSize() const {
    return contractCache.GetCacheSize() +
        txDiskPosCache.GetCacheSize() +
        contractRelatedKidCache.GetCacheSize() +
        contractDataCache.GetCacheSize() +
        contractAccountCache.GetCacheSize();
}

bool CContractDBCache::GetContractAccounts(const CRegID &scriptId, map<string, string> &mapAcc) {
    return false;
    /* TODO: GetContractAccounts
    return pBase->GetContractAccounts(scriptId, mapAcc);
    */
}

bool CContractDBCache::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
    return txDiskPosCache.GetData(txid, pos);
}

bool CContractDBCache::SetTxIndex(const uint256 &txid, const CDiskTxPos &pos) {
    return txDiskPosCache.SetData(txid, pos);
}

bool CContractDBCache::WriteTxIndexes(const vector<pair<uint256, CDiskTxPos> > &list) {
    for (auto it : list) {
        LogPrint("txindex", "txid:%s dispos: nFile=%d, nPos=%d nTxOffset=%d\n",
                it.first.GetHex(), it.second.nFile, it.second.nPos, it.second.nTxOffset);

        if (!txDiskPosCache.SetData(it.first, it.second))
            return false;
    }
    return true;
}

bool CContractDBCache::SetTxRelAccout(const uint256 &txid, const set<CKeyID> &relAccount) {
    return contractRelatedKidCache.SetData(txid, relAccount);
}

bool CContractDBCache::GetTxRelAccount(const uint256 &txid, set<CKeyID> &relAccount) {
    return contractRelatedKidCache.GetData(txid, relAccount);
}

bool CContractDBCache::EraseTxRelAccout(const uint256 &txid) { return contractRelatedKidCache.EraseData(txid); }

shared_ptr<CDBContractDatasGetter> CContractDBCache::CreateContractDatasGetter(
    const CRegID &contractRegid, const string &contractKeyPrefix, uint32_t count,
    const string &lastKey) {

    assert(contractDataCache.GetBasePtr() == nullptr && "only support top level cache");
    if (contractKeyPrefix.size() > CDBContractKey::MAX_KEY_SIZE) {
        LogPrint("ERROR", "CContractDBCache::CreateContractDatasGetter() contractKeyPrefix.size()=%u "
                 "exceeded the max size=%u", contractKeyPrefix.size(), CDBContractKey::MAX_KEY_SIZE);
        return nullptr;
    }
    auto prefix = make_pair(contractRegid.ToRawString(), CDBContractKey(contractKeyPrefix));
    return make_shared<CDBContractDatasGetter>(contractDataCache, prefix);
}