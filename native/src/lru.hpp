#ifndef __CCOMPLETE_LRU_HPP_
#define __CCOMPLETE_LRU_HPP_

/**
 * This file is part of cComplete (https://bitbucket.org/blue-dev/ccomplete).
 *
 * @copyright 2013, Robin Dietrich
 * @license Apache 2.0 <http://www.apache.org/licenses/LICENSE-2.0>
 * @license Public Domain <>
 */

#include <string>
#include <unordered_map>
#include <list>
#include <functional>

/** class providing a lru map */
template <typename K, typename V>
class lru {
    protected:
        /** typedef for a single map entry */
        typedef std::pair<K, V> EntryPair;
        /** pair cache */
        typedef std::list<EntryPair> Cache;
        /** map connecting cache + entries */
        typedef std::unordered_map<K, typename Cache::iterator> Map;
        /** callback function applied to each value purges */
        typedef std::function<void(V)> callback;
        
        /** cache lru list */
        Cache _cache;
        /** cache map for the list */
        Map _map;
        /** maximum number of entries */
        std::size_t _size;
        /** callback function */
        callback _cb;
    public:
        /** constructor, sets default container size of 10 */
        lru() : _cache(), _map(), _size(10), _cb(nullptr) {
        }
        
        /** destructor */
        ~lru() {
            _map.clear();
            _cache.clear();
        }
        
        /** changes number of maximum entries to s */
        void setSize(std::size_t s) {
            _size = s;
        }
        
        /** returns number of maximum entries */
        const std::size_t& getSize() {
            return _size;
        }
        
        /** sets the callback function invoked when freeing an entry */
        void setCallback(callback &&c) {
            _cb = c;
        }
        
        /** inserts a new key into the map */
        V insert(K key, V value) {
            _cache.push_front(std::make_pair(key, value));
            _map[key] = _cache.begin();
            
            while (_cache.size() > _size) {
                if (_cb != nullptr) 
                    _cb(_cache.back().second);
                
                _map.erase(_cache.back().first);
                _cache.pop_back();
            }
            
            return value;
        }
        
        /** checks if a key is cached */
        bool has(K key) {
            return _map.find(key) != _map.end();
        }
        
        /** returns a cached key */
        V retrieve(K key) {
            return _map[key]->second;    
        }
};

#endif /* __CCOMPLETE_LRU_HPP_ */