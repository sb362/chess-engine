#pragma once

#include <array>

namespace util
{

template <typename K, typename E>
class HashTable
{
public:
	using Key = K;
	using Entry = E;

	struct EntryWithKey
	{
		Key key = 0;
		Entry entry = {};
	};

	using Compare = bool (*)(const Entry &, const Entry &);

private:
	std::size_t nentries;
	EntryWithKey *entries;
	std::size_t hits, misses, successful_writes, failed_writes;
	std::uint8_t epoch;

public:
	HashTable(const std::size_t size_in_bytes)
		: nentries(size_in_bytes / sizeof(EntryWithKey)), entries(new EntryWithKey [nentries]),
		  hits(0), misses(0), successful_writes(0), failed_writes(0), epoch(0)
	{
	}

	~HashTable() { delete [] entries; }

	std::size_t total_hits() const { return hits; }
	std::size_t total_misses() const { return misses; }
	std::size_t total_probes() const { return hits + misses; }
	std::size_t total_successful_writes() const { return successful_writes; }
	std::size_t total_failed_writes() const { return failed_writes; }
	std::size_t total_writes() const { return successful_writes + failed_writes; }

	unsigned hit_rate() const
	{
		return (total_hits() * 100) / total_probes();
	}

	std::size_t entry_count() const { return nentries; }

	std::size_t size_in_bytes() const { return entry_count() * sizeof(EntryWithKey); }

	std::size_t used_entries() const
	{
		std::size_t used = 0;

		for (std::size_t i = 0; i < nentries; ++i)
			used += entries[i].key != 0;

		return used;
	}

	/**
	 * @brief Returns exact permillage of non-empty entries in the transposition table
	 * 
	 * @return unsigned 
	 */
	unsigned hashfull() const
	{
		return (1000 * used_entries()) / entry_count();
	}

	/**
	 * @brief Returns approximate permillage of non-empty entries in the transposition table
	 * 
	 * @return unsigned 
	 */
	unsigned hashfull_approx() const
	{
		std::size_t sample_size = util::min(entry_count(), 2048u);
		std::size_t used = 0;

		for (std::size_t i = 0; i < sample_size; ++i)
			used += entries[i].key != 0;

		return (used * 1000) / sample_size;
	}

	void reset_statistics()
	{
		hits = misses = successful_writes = failed_writes = 0;
		epoch = 0;
	}

	void resize(const std::size_t size_in_bytes)
	{
		nentries = size_in_bytes / sizeof(EntryWithKey);
		delete [] entries;
		entries = new EntryWithKey [nentries];

		reset_statistics();
	}

	void clear()
	{
		std::memset(entries, 0, size_in_bytes());

		reset_statistics();
	}

	void increment_epoch() { ++epoch; }
	std::uint8_t current_epoch() const { return epoch; }

	std::size_t index(const Key key) const { return key % nentries; }

	const Entry *probe(const Key key)
	{
		const std::size_t i = index(key);
		const Entry *entry = entries[i].key == key ? &entries[i].entry : nullptr;

		hits += entry != nullptr;
		misses += entry == nullptr;

		return entry;
	}

	bool write(const Key key, const Entry &entry, Compare compare)
	{
		const std::size_t i = index(key);
		EntryWithKey *entry_with_key = &entries[i];

		if (entry_with_key->key == key && !compare(entry_with_key->entry, entry))
			return ++failed_writes, false;

		entry_with_key->key = key;
		entry_with_key->entry = entry;
		return ++successful_writes, true;
	}
};

// Used to select the worst table entry when overwriting entries
template <typename E>
inline bool always_replace(const E &, const E &)
{
	// Always replace
	return true;
}

} // util
