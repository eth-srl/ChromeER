// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/engine/build_and_process_conflict_sets_command.h"

#include <string>
#include <sstream>
#include <vector>

#include "base/basictypes.h"
#include "base/format_macros.h"
#include "base/rand_util.h"
#include "chrome/browser/sync/engine/conflict_resolution_view.h"
#include "chrome/browser/sync/engine/syncer_util.h"
#include "chrome/browser/sync/engine/update_applicator.h"
#include "chrome/browser/sync/syncable/directory_manager.h"

namespace browser_sync {

using std::set;
using std::string;
using std::vector;

BuildAndProcessConflictSetsCommand::BuildAndProcessConflictSetsCommand() {}
BuildAndProcessConflictSetsCommand::~BuildAndProcessConflictSetsCommand() {}

void BuildAndProcessConflictSetsCommand::ModelChangingExecuteImpl(
    SyncerSession* session) {
  session->set_conflict_sets_built(BuildAndProcessConflictSets(session));
}

bool BuildAndProcessConflictSetsCommand::BuildAndProcessConflictSets(
    SyncerSession* session) {
  syncable::ScopedDirLookup dir(session->dirman(), session->account_name());
  if (!dir.good())
    return false;
  bool had_single_direction_sets = false;
  {  // Scope for transaction.
    syncable::WriteTransaction trans(dir, syncable::SYNCER, __FILE__, __LINE__);
    ConflictResolutionView conflict_view(session);
    BuildConflictSets(&trans, &conflict_view);
    had_single_direction_sets =
        ProcessSingleDirectionConflictSets(&trans, session);
    // We applied some updates transactionally, lets try syncing again.
    if (had_single_direction_sets)
      return true;
  }
  return false;
}

bool BuildAndProcessConflictSetsCommand::ProcessSingleDirectionConflictSets(
    syncable::WriteTransaction* trans, SyncerSession* const session) {
  bool rv = false;
  ConflictResolutionView conflict_view(session);
  set<ConflictSet*>::const_iterator all_sets_iterator;
  for(all_sets_iterator = conflict_view.ConflictSetsBegin();
      all_sets_iterator != conflict_view.ConflictSetsEnd() ; ) {
    const ConflictSet* conflict_set = *all_sets_iterator;
    CHECK(conflict_set->size() >= 2);
    // We scan the set to see if it consists of changes of only one type.
    ConflictSet::const_iterator i;
    size_t unsynced_count = 0, unapplied_count = 0;
    for (i = conflict_set->begin(); i != conflict_set->end(); ++i) {
      syncable::Entry entry(trans, syncable::GET_BY_ID, *i);
      CHECK(entry.good());
      if (entry.Get(syncable::IS_UNSYNCED))
        unsynced_count++;
      if (entry.Get(syncable::IS_UNAPPLIED_UPDATE))
        unapplied_count++;
    }
    if (conflict_set->size() == unsynced_count && 0 == unapplied_count) {
      LOG(INFO) << "Skipped transactional commit attempt.";
    } else if (conflict_set->size() == unapplied_count &&
               0 == unsynced_count &&
               ApplyUpdatesTransactionally(trans, conflict_set, session)) {
      rv = true;
    }
    ++all_sets_iterator;
  }
  return rv;
}

namespace {

void StoreLocalDataForUpdateRollback(syncable::Entry* entry,
                                     syncable::EntryKernel* backup) {
  CHECK(!entry->Get(syncable::IS_UNSYNCED)) << " Storing Rollback data for "
      "entry that's unsynced." << *entry ;
  CHECK(entry->Get(syncable::IS_UNAPPLIED_UPDATE)) << " Storing Rollback data "
      "for entry that's not an unapplied update." << *entry ;
  *backup = entry->GetKernelCopy();
}

class UniqueNameGenerator {
 public:
  void Initialize() {
    // To avoid name collisions we prefix the names with hex data derived from
    // 64 bits of randomness.
    int64 name_prefix = static_cast<int64>(base::RandUint64());
    name_stem_ = StringPrintf("%0" PRId64 "x.", name_prefix);
  }
  string StringNameForEntry(const syncable::Entry& entry) {
    CHECK(!name_stem_.empty());
    std::stringstream rv;
    rv << name_stem_ << entry.Get(syncable::ID);
    return rv.str();
  }
  PathString PathStringNameForEntry(const syncable::Entry& entry) {
    string name = StringNameForEntry(entry);
    return PathString(name.begin(), name.end());
  }

 private:
  string name_stem_;
};

bool RollbackEntry(syncable::WriteTransaction* trans,
                   syncable::EntryKernel* backup) {
  syncable::MutableEntry entry(trans, syncable::GET_BY_HANDLE,
                               backup->ref(syncable::META_HANDLE));
  CHECK(entry.good());

  if (!entry.Put(syncable::IS_DEL, backup->ref(syncable::IS_DEL)))
    return false;
  syncable::Name name = syncable::Name::FromEntryKernel(backup);
  if (!entry.PutParentIdAndName(backup->ref(syncable::PARENT_ID), name))
    return false;

  if (!backup->ref(syncable::IS_DEL)) {
    if (!entry.PutPredecessor(backup->ref(syncable::PREV_ID)))
      return false;
  }

  if (backup->ref(syncable::PREV_ID) != entry.Get(syncable::PREV_ID))
    return false;

  entry.Put(syncable::CTIME, backup->ref(syncable::CTIME));
  entry.Put(syncable::MTIME, backup->ref(syncable::MTIME));
  entry.Put(syncable::BASE_VERSION, backup->ref(syncable::BASE_VERSION));
  entry.Put(syncable::IS_DIR, backup->ref(syncable::IS_DIR));
  entry.Put(syncable::IS_DEL, backup->ref(syncable::IS_DEL));
  entry.Put(syncable::ID, backup->ref(syncable::ID));
  entry.Put(syncable::IS_UNAPPLIED_UPDATE,
            backup->ref(syncable::IS_UNAPPLIED_UPDATE));
  return true;
}

class TransactionalUpdateEntryPreparer {
 public:
  TransactionalUpdateEntryPreparer() {
    namegen_.Initialize();
  }

  void PrepareEntries(syncable::WriteTransaction* trans,
                      const vector<syncable::Id>* ids) {
    vector<syncable::Id>::const_iterator it;
    for (it = ids->begin(); it != ids->end(); ++it) {
      syncable::MutableEntry entry(trans, syncable::GET_BY_ID, *it);
      syncable::Name random_name(namegen_.PathStringNameForEntry(entry));
      CHECK(entry.PutParentIdAndName(trans->root_id(), random_name));
    }
  }

 private:
  UniqueNameGenerator namegen_;
  DISALLOW_COPY_AND_ASSIGN(TransactionalUpdateEntryPreparer);
};

}  // namespace

bool BuildAndProcessConflictSetsCommand::ApplyUpdatesTransactionally(
    syncable::WriteTransaction* trans,
    const vector<syncable::Id>* const update_set,
    SyncerSession* const session) {
  // The handles in the |update_set| order.
  vector<int64> handles;

  // Holds the same Ids as update_set, but sorted so that runs of adjacent
  // nodes appear in order.
  vector<syncable::Id> rollback_ids;
  rollback_ids.reserve(update_set->size());

  // Tracks what's added to |rollback_ids|.
  syncable::MetahandleSet rollback_ids_inserted_items;
  vector<syncable::Id>::const_iterator it;

  // 1. Build |rollback_ids| in the order required for successful rollback.
  //    Specifically, for positions to come out right, restoring an item
  //    requires that its predecessor in the sibling order is properly
  //    restored first.
  // 2. Build |handles|, the list of handles for ApplyUpdates.
  for (it = update_set->begin(); it != update_set->end(); ++it) {
    syncable::Entry entry(trans, syncable::GET_BY_ID, *it);
    SyncerUtil::AddPredecessorsThenItem(trans, &entry,
        syncable::IS_UNAPPLIED_UPDATE, &rollback_ids_inserted_items,
        &rollback_ids);
    handles.push_back(entry.Get(syncable::META_HANDLE));
  }
  DCHECK_EQ(rollback_ids.size(), update_set->size());
  DCHECK_EQ(rollback_ids_inserted_items.size(), update_set->size());

  // 3. Store the information needed to rollback if the transaction fails.
  // Do this before modifying anything to keep the next/prev values intact.
  vector<syncable::EntryKernel> rollback_data(rollback_ids.size());
  for (size_t i = 0; i < rollback_ids.size(); ++i) {
    syncable::Entry entry(trans, syncable::GET_BY_ID, rollback_ids[i]);
    StoreLocalDataForUpdateRollback(&entry, &rollback_data[i]);
  }

  // 4. Use the preparer to move things to an initial starting state where no
  // names collide, and nothing in the set is a child of anything else.  If
  // we've correctly calculated the set, the server tree is valid and no
  // changes have occurred locally we should be able to apply updates from this
  // state.
  TransactionalUpdateEntryPreparer preparer;
  preparer.PrepareEntries(trans, update_set);

  // 5. Use the usual apply updates from the special start state we've just
  // prepared.
  UpdateApplicator applicator(session, handles.begin(), handles.end());
  while (applicator.AttemptOneApplication(trans)) {
    // Keep going till all updates are applied.
  }
  if (!applicator.AllUpdatesApplied()) {
    LOG(ERROR) << "Transactional Apply Failed, Rolling back.";
    // We have to move entries into the temp dir again. e.g. if a swap was in a
    // set with other failing updates, the swap may have gone through, meaning
    // the roll back needs to be transactional. But as we're going to a known
    // good state we should always succeed.
    preparer.PrepareEntries(trans, update_set);

    // Rollback all entries.
    for (size_t i = 0; i < rollback_data.size(); ++i) {
      CHECK(RollbackEntry(trans, &rollback_data[i]));
    }
    return false;  // Don't save progress -- we just undid it.
  }
  applicator.SaveProgressIntoSessionState();
  return true;
}

void BuildAndProcessConflictSetsCommand::BuildConflictSets(
    syncable::BaseTransaction* trans,
    ConflictResolutionView* view) {
  view->CleanupSets();
  set<syncable::Id>::iterator i = view->CommitConflictsBegin();
  while (i != view->CommitConflictsEnd()) {
    syncable::Entry entry(trans, syncable::GET_BY_ID, *i);
    CHECK(entry.good());
    if (!entry.Get(syncable::IS_UNSYNCED) &&
        !entry.Get(syncable::IS_UNAPPLIED_UPDATE)) {
      // This can happen very rarely. It means we had a simply conflicting item
      // that randomly committed. We drop the entry as it's no longer
      // conflicting.
      view->EraseCommitConflict(i++);
      continue;
    }
    if (entry.ExistsOnClientBecauseDatabaseNameIsNonEmpty() &&
       (entry.Get(syncable::IS_DEL) || entry.Get(syncable::SERVER_IS_DEL))) {
       // If we're deleted on client or server we can't be in a complex set.
      ++i;
      continue;
    }
    bool new_parent =
        entry.Get(syncable::PARENT_ID) != entry.Get(syncable::SERVER_PARENT_ID);
    bool new_name = 0 != syncable::ComparePathNames(entry.GetSyncNameValue(),
                                          entry.Get(syncable::SERVER_NAME));
    if (new_parent || new_name)
      MergeSetsForNameClash(trans, &entry, view);
    if (new_parent)
      MergeSetsForIntroducedLoops(trans, &entry, view);
    MergeSetsForNonEmptyDirectories(trans, &entry, view);
    ++i;
  }
}

void BuildAndProcessConflictSetsCommand::MergeSetsForNameClash(
    syncable::BaseTransaction* trans, syncable::Entry* entry,
    ConflictResolutionView* view) {
  PathString server_name = entry->Get(syncable::SERVER_NAME);
  // Uncommitted entries have no server name. We trap this because the root
  // item has a null name and 0 parentid.
  if (server_name.empty())
    return;
  syncable::Id conflicting_id =
      SyncerUtil::GetNameConflictingItemId(
          trans, entry->Get(syncable::SERVER_PARENT_ID), server_name);
  if (syncable::kNullId != conflicting_id)
    view->MergeSets(entry->Get(syncable::ID), conflicting_id);
}

void BuildAndProcessConflictSetsCommand::MergeSetsForIntroducedLoops(
    syncable::BaseTransaction* trans, syncable::Entry* entry,
    ConflictResolutionView* view) {
  // This code crawls up from the item in question until it gets to the root
  // or itself. If it gets to the root it does nothing. If it finds a loop all
  // moved unsynced entries in the list of crawled entries have their sets
  // merged with the entry.
  // TODO(sync): Build test cases to cover this function when the argument list
  // has settled.
  syncable::Id parent_id = entry->Get(syncable::SERVER_PARENT_ID);
  syncable::Entry parent(trans, syncable::GET_BY_ID, parent_id);
  if (!parent.good()) {
    return;
  }
  // Don't check for loop if the server parent is deleted.
  if (parent.Get(syncable::IS_DEL))
    return;
  vector<syncable::Id> conflicting_entries;
  while (!parent_id.IsRoot()) {
    syncable::Entry parent(trans, syncable::GET_BY_ID, parent_id);
    if (!parent.good()) {
      LOG(INFO) << "Bad parent in loop check, skipping. Bad parent id: "
        << parent_id << " entry: " << *entry;
      return;
    }
    if (parent.Get(syncable::IS_UNSYNCED) &&
        entry->Get(syncable::PARENT_ID) !=
            entry->Get(syncable::SERVER_PARENT_ID))
      conflicting_entries.push_back(parent_id);
    parent_id = parent.Get(syncable::PARENT_ID);
    if (parent_id == entry->Get(syncable::ID))
      break;
  }
  if (parent_id.IsRoot())
    return;
  for (size_t i = 0; i < conflicting_entries.size(); i++) {
    view->MergeSets(entry->Get(syncable::ID), conflicting_entries[i]);
  }
}

namespace {

class ServerDeletedPathChecker {
 public:
  bool CausingConflict(const syncable::Entry& e,
                       const syncable::Entry& log_entry) {
    CHECK(e.good()) << "Missing parent in path of: " << log_entry;
    if (e.Get(syncable::IS_UNAPPLIED_UPDATE) &&
        e.Get(syncable::SERVER_IS_DEL)) {
      CHECK(!e.Get(syncable::IS_DEL)) << " Inconsistency in local tree. "
                            "syncable::Entry: " << e << " Leaf: " << log_entry;
      return true;
    } else {
      CHECK(!e.Get(syncable::IS_DEL)) << " Deleted entry has children. "
                            "syncable::Entry: " << e << " Leaf: " << log_entry;
      return false;
    }
  }
  // returns 0 if we should stop investigating the path.
  syncable::Id GetAndExamineParent(syncable::BaseTransaction* trans,
                            syncable::Id id,
                            syncable::Id check_id,
                            const syncable::Entry& log_entry) {
    syncable::Entry parent(trans, syncable::GET_BY_ID, id);
    CHECK(parent.good()) << "Tree inconsitency, missing id" << id << " "
        << log_entry;
    syncable::Id parent_id = parent.Get(syncable::PARENT_ID);
    CHECK(parent_id != check_id) << "Loop in dir tree! "
      << log_entry << " " << parent;
    return parent_id;
  }
};

class LocallyDeletedPathChecker {
 public:
  bool CausingConflict(const syncable::Entry& e,
                       const syncable::Entry& log_entry) {
    return e.good() && e.Get(syncable::IS_DEL) && e.Get(syncable::IS_UNSYNCED);
  }
  // returns 0 if we should stop investigating the path.
  syncable::Id GetAndExamineParent(syncable::BaseTransaction* trans,
                                   syncable::Id id,
                                   syncable::Id check_id,
                                   const syncable::Entry& log_entry) {
    syncable::Entry parent(trans, syncable::GET_BY_ID, id);
    if (!parent.good())
      return syncable::kNullId;
    syncable::Id parent_id = parent.Get(syncable::PARENT_ID);
    if (parent_id == check_id)
      return syncable::kNullId;
    return parent_id;
  }
};

template <typename Checker>
void CrawlDeletedTreeMergingSets(syncable::BaseTransaction* trans,
                                 const syncable::Entry& entry,
                                 ConflictResolutionView* view,
                                 Checker checker) {
  syncable::Id parent_id = entry.Get(syncable::PARENT_ID);
  syncable::Id double_step_parent_id = parent_id;
  // This block builds sets where we've got an entry in a directory the server
  // wants to delete.
  //
  // Here we're walking up the tree to find all entries that the pass checks
  // deleted. We can be extremely strict here as anything unexpected means
  // invariants in the local hierarchy have been broken.
  while (!parent_id.IsRoot()) {
    if (!double_step_parent_id.IsRoot()) {
      // Checks to ensure we don't loop.
      double_step_parent_id = checker.GetAndExamineParent(
          trans, double_step_parent_id, parent_id, entry);
      double_step_parent_id = checker.GetAndExamineParent(
          trans, double_step_parent_id, parent_id, entry);
    }
    syncable::Entry parent(trans, syncable::GET_BY_ID, parent_id);
    if (checker.CausingConflict(parent, entry))
      view->MergeSets(entry.Get(syncable::ID), parent.Get(syncable::ID));
    else
      break;
    parent_id = parent.Get(syncable::PARENT_ID);
  }
}

}  // namespace

void BuildAndProcessConflictSetsCommand::MergeSetsForNonEmptyDirectories(
    syncable::BaseTransaction* trans, syncable::Entry* entry,
    ConflictResolutionView* view) {
  if (entry->Get(syncable::IS_UNSYNCED) && !entry->Get(syncable::IS_DEL)) {
    ServerDeletedPathChecker checker;
    CrawlDeletedTreeMergingSets(trans, *entry, view, checker);
  }
  if (entry->Get(syncable::IS_UNAPPLIED_UPDATE) &&
      !entry->Get(syncable::SERVER_IS_DEL)) {
    syncable::Entry parent(trans, syncable::GET_BY_ID,
                           entry->Get(syncable::SERVER_PARENT_ID));
    syncable::Id parent_id = entry->Get(syncable::SERVER_PARENT_ID);
    if (!parent.good())
      return;
    LocallyDeletedPathChecker checker;
    if (!checker.CausingConflict(parent, *entry))
      return;
    view->MergeSets(entry->Get(syncable::ID), parent.Get(syncable::ID));
    CrawlDeletedTreeMergingSets(trans, parent, view, checker);
  }
}

}  // namespace browser_sync
