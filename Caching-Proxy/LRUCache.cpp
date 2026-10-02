#include "LRUCache.hpp"

int LRUCache::get(int key) {
	const auto it = cache.find(key);
	if (it == cache.cend())
		return -1;
	items.splice(items.cbegin(), items, it->second);
	return it->second->second;
}

void LRUCache::put(int key, int value) {
	const auto it = cache.find(key);

	if (it != cache.cend()) {
		items.splice(items.cbegin(), items, it->second);
		it->second->second = value;
		return;
	}

	if (items.size() == capacity) {
		const int keyToDelete = items.back().first;
		items.pop_back();
		cache.erase(keyToDelete);
	}

	items.emplace_front(key, value);
	cache[key] = items.begin();
}

void LRUCache::clear() noexcept {
	items.clear();
	cache.clear();
	capacity = 0;
}
