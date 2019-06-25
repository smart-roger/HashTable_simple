#include <random>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>

template <typename T>
struct hasher {
    std::size_t my_hash (T value);
};

template<>
struct hasher<uint8_t> {
    std::size_t my_hash(uint8_t value) {return value;}
};

//https://sohabr.net/habr/post/219139/
template<>
struct hasher<uint64_t> {
    std::size_t my_hash (uint64_t value) {
        std::size_t hash = 0;

        for (int idx=0; idx<8; ++idx)
        {
            hash += static_cast<uint8_t>(value&0xFF);
            hash += (hash << 10);
            hash ^= (hash >> 6);
            value >>=8;
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        return hash;
    }
};

//https://ru.stackoverflow.com/questions/865517/c-%D0%A5%D0%B5%D1%88-%D0%BE%D1%82-%D1%81%D1%82%D1%80%D0%BE%D0%BA%D0%B8-%D0%BD%D1%83%D0%B6%D0%B5%D0%BD-%D0%B0%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC
template<>
struct hasher<std::string> {
    std::size_t my_hash (std::string value) {
        uint32_t num = 352654597;
        uint32_t num2 = num;

        for (std::size_t i = 0; i < value.length(); i += 4) {
            int ptr0 = value[i] << 16;
            if (i + 1 < value.length())
                ptr0 |= value[i + 1];

            num = (num << 5) + num + (num >> 27) ^ ptr0;

                if (i + 2 < value.length()) {
                    int ptr1 = value[i + 2] << 16;
                    if (i + 3 < value.length())
                        ptr1 |= value[i + 3];
                    num2 = (num2 << 5) + num2 + (num2 >> 27) ^ ptr1;
                }
            }

            return num + num2 * 1566083941;
    }
};
template<typename T, int load_factor_perc, class Hasher = hasher<T>>
class HashTable {
    using Chain = std::list<T>;
    std::vector<Chain> table;
    Hasher _hasher;

public:
    HashTable(uint8_t capacity):
            used(0),
            capacity(capacity) {
        table.resize(capacity * load_factor_perc / 100);
    }

    ~HashTable(){}

    void insert(const T& value) {
        if(used == capacity) {
            resize();
        }

        Chain& chain_with_key = get_chain(value);
        chain_with_key.push_back(value);
        ++used;
    }

    bool contains(const T& value){
        Chain& chain_with_key = get_chain(value);
        return std::find(chain_with_key.begin(), chain_with_key.end(), value) != chain_with_key.end();
    }

    void remove(const T& value) {
        Chain& chain_with_key = get_chain(value);
        auto iter = std::find(chain_with_key.begin(), chain_with_key.end(), value);
        if (iter!=chain_with_key.end()) {
            chain_with_key.erase(iter);
            --used;
        }
    }
    template<typename U, int load_factor>
    friend std::ostream& operator<< (std::ostream &out, const HashTable<U, load_factor> &table);
private:
    Chain& get_chain(const T& value) {
        std::size_t key = _hasher.my_hash(value) % table.size();
        return table[key];
    }

    void resize() {
        std::size_t new_size = capacity * 3;
        std::vector<Chain> new_table;
        new_table.resize(new_size);
        for(auto& chain: table) {
            for(const auto& val: chain) {
                auto key = _hasher.my_hash(val) % new_size;
                //  Creating copy to prevent exception - we won't lost data moved in new table;
                new_table[key].push_back(val);
            }
        }
        std::swap(table, new_table);
        capacity = new_size;
    }

    std::size_t used;
    std::size_t capacity;
};

template<typename T, int load_factor_perc>
std::ostream& operator<< (std::ostream &out, const HashTable<T, load_factor_perc> &table) {
    out << "Hash Table. Chains:" << table.table.size() <<  "\tElements: " << table.used << std::endl;
    std::size_t counter = 0;
    for(const auto& chain : table.table) {
        out << counter <<":\t";
        for (const auto& value : chain) {
            out << value << "\t";
        }
        out << std::endl;
        ++counter;
    }
    return out;
}

int main()
{
    std::mt19937 mt_rand(0);

    // test uint_8
    {
        HashTable<uint8_t, 75> hash_table_uint8(5);
        std::cout << hash_table_uint8;

        for (uint8_t i=0; i<5 ; ++i) {
            hash_table_uint8.insert(i);
        }
        std::cout << hash_table_uint8 << std::endl;

        for (int i=0; i<20 ; ++i) {
            hash_table_uint8.insert(static_cast<uint8_t>(mt_rand()));
        }
        std::cout << hash_table_uint8 << std::endl;

        for (uint8_t i=0; i<50 ; ++i) {
            if (hash_table_uint8.contains(i))
                hash_table_uint8.remove(i);
        }
        std::cout << hash_table_uint8 << std::endl;
    }

    //test uint64
    {
        HashTable<uint64_t, 100> hash_table_uint64(5);
        std::cout << hash_table_uint64;
        for (uint8_t i=0; i<5 ; ++i) {
            hash_table_uint64.insert(i);
        }
        std::cout << hash_table_uint64 << std::endl;

        for (int i=0; i<20 ; ++i) {
            hash_table_uint64.insert(static_cast<uint8_t>(mt_rand()));
        }
        std::cout << hash_table_uint64 << std::endl;

        for (uint64_t i=0; i<100 ; ++i) {
            if (hash_table_uint64.contains(i))
                hash_table_uint64.remove(i);
        }
        std::cout << hash_table_uint64 << std::endl;
    }

    //test string
    {
        std::vector<std::string> test_data_first = {"The", "Ultimate", "question", "of", "Life", "the", "Universe", "and", "Everything"};
        std::vector<std::string> test_data_second = {"Nothing", "everything", "anything", "something", "if", "you", "have", "nothing", "then",
                                                    "you", "have", "the", "freedom", "to", "do", "anything", "without", "the", "fear",
                                                     "of", "losing", "something"};
        HashTable<std::string, 50> hash_table_string(5);
        std::cout << hash_table_string;
        for (const auto& val : test_data_first) {
            hash_table_string.insert(val);
        }
        std::cout << hash_table_string << std::endl;

        for (const auto& val : test_data_second) {
            hash_table_string.insert(val);
        }
        std::cout << hash_table_string << std::endl;

        std::vector<std::string> test_data_remove = {"the", "anything", "the", "nothing", "of", "anybody", "not", "", "of"};
        for (const auto& data : test_data_remove) {
            if (hash_table_string.contains(data))
                hash_table_string.remove(data);
        }
        std::cout << hash_table_string << std::endl;
    }

    return 0;
}
