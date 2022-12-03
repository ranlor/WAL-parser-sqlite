#include <cstdint>
#include <memory>
#include <iterator>
#include <cstring>
#pragma once
namespace wal {
    // simple size protected array
    // that can be allocated to fixed size in runtime and has only one
    // allocation on creation (unlike vector's allocation policy) and
    // can be done in runtime (unlike array that can be allocated in
    // compile time)
    template<typename T>
    class FixedRuntimeArray
    {
        public:

            class iterator  : public std::iterator<
                                std::input_iterator_tag,// iterator_category
                                T,                      // value_type
                                std::ptrdiff_t,         // difference_type
                                T*,                     // pointer
                                T&                      // reference
                                >{
                public:
                    explicit iterator(T* ptr, size_t size) :
                        _ptr(ptr),
                        _size(size),
                        _count(0)
                    {}

                    iterator& operator++() // prefix ++
                    {
                        if ( nullptr == _ptr ) { return *this; }
                        _ptr++;
                        _count++;
                        if (_count >= _size) { _ptr = nullptr; }
                        return *this;
                    }

                    iterator operator++(int) // postfix ++
                    {
                        iterator retval = *this;
                        ++(*this);
                        return retval;
                    }

                    iterator operator+=(std::ptrdiff_t offset)
                    {
                        if ( nullptr == _ptr ) { return *this; }
                        _ptr+=offset;
                        _count+=offset;
                        if (_count >= _size) { _ptr = nullptr; }
                        return *this;
                    }

                    iterator operator+(std::ptrdiff_t offset) const
                    {
                        iterator retval = *this;
                        retval+=offset;
                        return retval;
                    }


                    bool operator==(iterator other) const
                    {
                        return _ptr == other._ptr;
                    }

                    bool operator!=(iterator other) const
                    {
                        return !(*this == other);
                    }

                    T& operator*() const
                    {
                        if ( nullptr == _ptr ) { throw std::out_of_range("while trying to access data, it's out of range"); }
                        return *_ptr;
                    }

                    T& operator[](std::ptrdiff_t index) const
                    {
                        if ( nullptr == _ptr ) { throw std::out_of_range("while trying to access data, it's out of range"); }
                        return *(_ptr+index);
                    }

                private:
                    T* _ptr;
                    size_t _size;
                    size_t _count;
            };

            FixedRuntimeArray(size_t size,T value = 0):
                _data(std::make_unique<T[]>(size)),
                _size(size)
            {
                for (size_t i=0; i<size; ++i)
                {
                    _data[i] = value;
                }
            }

            FixedRuntimeArray(std::initializer_list<T> elements):
                _data(std::make_unique<T[]>(elements.size())),
                _size(elements.size())
            {
                size_t i = 0;
                for (T e : elements)
                {
                    _data[i++] = e;
                }
            }

            FixedRuntimeArray(const FixedRuntimeArray<T>& arr):
                _data(std::make_unique<T[]>(arr.size())),
                _size(arr.size())
            {
                for (size_t i=0; i<_size; ++i)
                {
                    _data[i] = arr[i];
                }
            }

            FixedRuntimeArray(const FixedRuntimeArray<T>&& arr):
                _data(std::make_unique<T[]>(arr.size())),
                _size(arr.size())
            {
                for (size_t i=0; i<_size; ++i)
                {
                    _data[i] = arr[i];
                }
            }

            FixedRuntimeArray(const std::vector<T>& arr):
                _data(std::make_unique<T[]>(arr.size())),
                _size(arr.size())
            {
                for (size_t i=0; i<_size; ++i)
                {
                    _data[i] = arr[i];
                }
            }

            FixedRuntimeArray(FixedRuntimeArray<T>::iterator& it, size_t size):
                _data(std::make_unique<T[]>(size)),
                _size(size)
            {
                for (size_t i=0; i<_size; ++i)
                {
                    _data[i] = *(it++);
                }
            }

            ~FixedRuntimeArray() = default;

            iterator begin() {return iterator(_data.get(), _size);}
            iterator end() {return iterator(nullptr, 0);}

            iterator begin() const {return iterator(_data.get(), _size);}
            iterator end() const {return iterator(nullptr, 0);}

            T& operator[](std::size_t i)
            {
                if (i >= _size) { throw std::out_of_range("while trying to access data with [] operator, it's out of range"); }
                return _data[i];
            }

            const T& operator[](std::size_t i) const
            {
                if (i >= _size) { throw std::out_of_range("while trying to access data with [] operator, it's out of range"); }
                return (_data)[i];
            }

            size_t size() const { return _size; }

            T* data() { return _data.get(); }

            const T* data() const { return _data.get(); }

        private:
            std::unique_ptr<T[]> _data;
            size_t _size;
    };
}