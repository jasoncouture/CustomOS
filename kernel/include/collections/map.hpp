#pragma once
#include <stdint.h>
#include <memory/heap.hpp>

template <class TKey>
class KeyHasher
{
public:
    virtual uint32_t ComputeHash(TKey key)
    {
        uint32_t hashCode = 17;
        uint8_t *keyBuffer = (uint8_t *)&key;
        for (auto x = sizeof(TKey); x > 0; x--)
        {
            hashCode = hashCode * 31 + *(keyBuffer + x);
        }

        return hashCode;
    }

    virtual bool Equal(TKey left, TKey right)
    {
        return left == right;
    }
};

template <class TKey, class TValue>
struct MapEntry
{
    TKey Key;
    TValue Value;
    MapEntry *Next;
    MapEntry *Previous;
};

template <class TKey, class TValue>
class Map
{
public:
    Map(KeyHasher<TKey> *hasher, bool freeHasherOnDelete = false, uint64_t capacity = 256)
    {
        this->hasher = hasher;
        this->freeHasher = freeHasherOnDelete;
        this->bucketCount = capacity;
        this->buckets = (MapEntry<TKey, TValue>*)calloc(this->bucketCount, sizeof(MapEntry<TKey, TValue>));
    }

    bool TryGet(TKey key, TValue *result)
    {
        auto bucketNumber = this->hasher->ComputeHash(key) % this->bucketCount;
        auto bucketStart = this->buckets + bucketNumber;
        auto currentBucket = bucketStart->Next;
        while (currentBucket != NULL)
        {
            if (this->hasher->Equal(currentBucket->Key, key))
            {
                (*result) = currentBucket->Value;
                return true;
            }
            currentBucket = currentBucket->Next;
        }
        return false;
    }

    bool Add(TKey key, TValue value)
    {
        auto bucketNumber = this->hasher->ComputeHash(key) % this->bucketCount;
        auto bucketStart = this->buckets + bucketNumber;
        auto currentBucket = bucketStart->Next;
        auto lastBucket = currentBucket;
        while (currentBucket != NULL)
        {
            if (this->hasher->Equal(currentBucket->Key, key))
                return false;
            lastBucket = currentBucket;
            currentBucket = currentBucket->Next;
        }
        if(lastBucket == NULL) 
        {
            lastBucket = bucketStart;
        }
        lastBucket->Next = this->CreateBucket(key, value);
        lastBucket->Next->Previous = lastBucket;
        return true;
    }

    bool Remove(TKey key)
    {
        auto bucketNumber = this->hasher->ComputeHash(key) % this->bucketCount;
        auto bucketStart = this->buckets + bucketNumber;
        auto currentBucket = bucketStart->Next;
        auto lastBucket = currentBucket;
        while (currentBucket != NULL)
        {
            if (this->hasher->Equal(currentBucket->Key, key))
            {
                currentBucket->Previous->Next = currentBucket->Next;
                if (currentBucket->Next)
                {
                    currentBucket->Next->Previous = currentBucket->Previous;
                }
                delete currentBucket;
                return true;
            }
            lastBucket = currentBucket;
            currentBucket = currentBucket->Next;
        }
        return false;
    }

    Map() : Map(new KeyHasher<TKey>(), true) {}

    ~Map()
    {
        if (hasher && this->freeHasher)
        {
            delete hasher;
            hasher = NULL;
        }
        if (this->buckets)
        {
            // Free all the buckets.
            for (auto x = 0; x < bucketCount; x++)
            {
                auto bucket = this->buckets + x;
                if (bucket)
                {

                    while (bucket->Next)
                    {
                        auto next = bucket->Next;
                        delete bucket;
                        bucket = next;
                    }
                }
            }
            delete this->buckets;
            this->buckets = NULL;
        }
    }

private:
    MapEntry<TKey, TValue> *CreateBucket(TKey key, TValue value)
    {
        MapEntry<TKey, TValue> *newEntry = new MapEntry<TKey, TValue>();
        newEntry->Key = key;
        newEntry->Value = value;
        newEntry->Previous = NULL;
        newEntry->Next = NULL;
        return newEntry;
    }

    bool freeHasher;
    uint64_t bucketCount;
    MapEntry<TKey, TValue> *buckets;
    KeyHasher<TKey> *hasher;
};