#include "db_table.h"

TCTable::TCTable(RAIILock& lock,
                 const std::shared_ptr<InternalEntryComparator>& comparator,
                 const uint64_t first_entry_id)
    : invalid_key_(new char(0)),
      table_(comparator, invalid_key_),
      mem_allocator_(std::make_shared<MemAllocator>()),
      query_allocator_(std::make_shared<MemAllocator>()),
      table_lock_(lock),
      entry_id_(first_entry_id) {}

TCTable::~TCTable() {
  if (invalid_key_ != nullptr)
    delete invalid_key_;
}

const Sequence TCTable::Get(const Sequence& key) const {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  char* internal_entry = query_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  // The value, ID, and op_type are invalid
  Status enc = InternalEntry::EncodeInternal(
      key, Sequence(), static_cast<uint64_t>(0) - 1, InternalEntry::kDelete,
      internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    // const SkipListNode<const char*>* the_node = table_.Get(internal_entry);
    auto the_node = table_.Get(internal_entry);
    table_lock_.Unlock();

    if (the_node != nullptr) {
      if (InternalEntry::EntryOpType(the_node->key_) ==
          InternalEntry::kInsert) {
        // return new Sequence(the_node->key_, 0);
        return InternalEntry::EntryValue(the_node->key_);
      }
    }
  }

  return Sequence();
}

const Sequence TCTable::Get(const char* internal_entry) const {
  table_lock_.Lock();
  // const SkipListNode<const char*>* the_node = table_.Get(internal_entry);
  auto the_node = table_.Get(internal_entry);
  table_lock_.Unlock();

  if (the_node != nullptr) {
    if (InternalEntry::EntryOpType(the_node->key_) == InternalEntry::kInsert) {
      // return new Sequence(the_node->key_, 0);
      return InternalEntry::EntryValue(the_node->key_);
    }
  }

  return Sequence();
}

Status TCTable::Insert(const Sequence& key, const Sequence& value) {
  // See InternalEntry.h for format info
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9 +
                        coding::SizeOfVarint(value.size()) + value.size();

  table_lock_.Lock();
  // TCTable is in charge of allocating and managing the memory
  // The underlying SkipList DOES NOT hold any data resources
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  Status enc = InternalEntry::EncodeInternal(
      key, value, entry_id_++, InternalEntry::OpType::kInsert, internal_entry);
  if (enc.StatusNoError()) {
    table_lock_.Lock();
    table_.Insert(internal_entry);
    table_lock_.Unlock();
  }

  return Status::NoError();
}

Status TCTable::Delete(const Sequence& key) {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  char* internal_entry = mem_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  Status enc = InternalEntry::EncodeInternal(key, Sequence(), entry_id_++,
                                             InternalEntry::OpType::kDelete,
                                             internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    table_.Insert(internal_entry);
    table_lock_.Unlock();
  }

  return Status::NoError();
}

bool TCTable::ContainsKey(const Sequence& key) const {
  uint64_t entry_size = coding::SizeOfVarint(key.size()) + key.size() + 9;

  table_lock_.Lock();
  // char* internal_entry = mem_allocator_->Allocate(entry_size);
  char* internal_entry = query_allocator_->Allocate(entry_size);
  table_lock_.Unlock();

  // The value, ID, and op_type are invalid
  Status enc = InternalEntry::EncodeInternal(
      key, Sequence(), static_cast<uint64_t>(0) - 1, InternalEntry::kDelete,
      internal_entry);

  if (enc.StatusNoError()) {
    table_lock_.Lock();
    // const SkipListNode<const char*>* the_node = table_.Get(internal_entry);
    auto the_node = table_.Get(internal_entry);
    table_lock_.Unlock();

    if (the_node != nullptr) {
      if (InternalEntry::EntryOpType(the_node->key_) == InternalEntry::kInsert)
        return true;
    }
  }

  return false;
}