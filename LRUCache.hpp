#pragma once

// Credits: https://medium.com/@shahjui2000/the-o-1-solution-mastering-the-lru-cache-with-modern-c-416afc0bfe83

#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

class LRUCache {
private:
	using ListIterator = std::list<std::pair<int, int>>::iterator;

	std::size_t capacity{};
	std::list<std::pair<int, int>> items{};
	std::unordered_map<int, ListIterator> cache{};

public:
	explicit LRUCache(std::size_t cap) : capacity{ cap } {}

	int get(int key);
	void put(int key, int value);
	void clear() noexcept;
};