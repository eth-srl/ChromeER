// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sync/internal_api/public/attachments/in_memory_attachment_store.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/memory/scoped_ptr.h"

namespace syncer {

InMemoryAttachmentStore::InMemoryAttachmentStore(
    const scoped_refptr<base::SingleThreadTaskRunner>& callback_task_runner)
    : callback_task_runner_(callback_task_runner) {
  // Object is created on one thread but used on another.
  DetachFromThread();
}

InMemoryAttachmentStore::~InMemoryAttachmentStore() {
}

void InMemoryAttachmentStore::Init(const InitCallback& callback) {
  DCHECK(CalledOnValidThread());
  callback_task_runner_->PostTask(FROM_HERE, base::Bind(callback, SUCCESS));
}

void InMemoryAttachmentStore::Read(const AttachmentIdList& ids,
                                   const ReadCallback& callback) {
  DCHECK(CalledOnValidThread());
  Result result_code = SUCCESS;
  AttachmentIdList::const_iterator id_iter = ids.begin();
  AttachmentIdList::const_iterator id_end = ids.end();
  scoped_ptr<AttachmentMap> result_map(new AttachmentMap);
  scoped_ptr<AttachmentIdList> unavailable_attachments(new AttachmentIdList);
  for (; id_iter != id_end; ++id_iter) {
    const AttachmentId& id = *id_iter;
    syncer::AttachmentMap::iterator attachment_iter =
        attachments_.find(*id_iter);
    if (attachment_iter != attachments_.end()) {
      const Attachment& attachment = attachment_iter->second;
      result_map->insert(std::make_pair(id, attachment));
    } else {
      unavailable_attachments->push_back(id);
    }
  }
  if (!unavailable_attachments->empty()) {
    result_code = UNSPECIFIED_ERROR;
  }
  callback_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(callback,
                 result_code,
                 base::Passed(&result_map),
                 base::Passed(&unavailable_attachments)));
}

void InMemoryAttachmentStore::Write(const AttachmentList& attachments,
                                    const WriteCallback& callback) {
  DCHECK(CalledOnValidThread());
  AttachmentList::const_iterator iter = attachments.begin();
  AttachmentList::const_iterator end = attachments.end();
  for (; iter != end; ++iter) {
    attachments_.insert(std::make_pair(iter->GetId(), *iter));
  }
  callback_task_runner_->PostTask(FROM_HERE, base::Bind(callback, SUCCESS));
}

void InMemoryAttachmentStore::Drop(const AttachmentIdList& ids,
                                   const DropCallback& callback) {
  DCHECK(CalledOnValidThread());
  Result result = SUCCESS;
  AttachmentIdList::const_iterator ids_iter = ids.begin();
  AttachmentIdList::const_iterator ids_end = ids.end();
  for (; ids_iter != ids_end; ++ids_iter) {
    AttachmentMap::iterator attachments_iter = attachments_.find(*ids_iter);
    if (attachments_iter != attachments_.end()) {
      attachments_.erase(attachments_iter);
    }
  }
  callback_task_runner_->PostTask(FROM_HERE, base::Bind(callback, result));
}

void InMemoryAttachmentStore::ReadMetadata(
    const AttachmentIdList& ids,
    const ReadMetadataCallback& callback) {
  // TODO(stanisc): implement this.
  NOTIMPLEMENTED();
}

void InMemoryAttachmentStore::ReadAllMetadata(
    const ReadMetadataCallback& callback) {
  // TODO(stanisc): implement this.
  NOTIMPLEMENTED();
}

}  // namespace syncer
