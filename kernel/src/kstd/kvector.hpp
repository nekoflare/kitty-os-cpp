#include <initializer_list>

namespace kstd
{
    template <typename T>
    class vector
    {
    private:
        T* array;         // Pointer to the array
        size_t size;      // Current number of elements
        size_t capacity;  // Current capacity of the array

        // Function to resize the array when it runs out of space
        void resize()
        {
            capacity *= 2;
            T* newArray = new T[capacity];
            for (size_t i = 0; i < size; ++i)
            {
                newArray[i] = array[i];
            }
            delete[] array;
            array = newArray;
        }

    public:
        // Constructor
        vector() : size(0), capacity(1)
        {
            array = new T[capacity];
        }

        // Initializer list constructor
        vector(std::initializer_list<T> initList) : size(initList.size()), capacity(initList.size())
        {
            array = new T[capacity];
            size_t index = 0;
            for (const T& value : initList)
            {
                array[index++] = value;
            }
        }

        // Destructor
        ~vector()
        {
            delete[] array;
        }

        void reserve(size_t newCapacity) {
            if (newCapacity <= capacity) return; // No need to reallocate

            T* newData = new T[newCapacity];
            for (size_t i = 0; i < size; ++i) {
                newData[i] = array[i]; // Move existing elements
            }
            delete[] array; // Delete old data
            array = newData;
            capacity = newCapacity;
        }

        // Get the current size of the vector
        size_t getSize() const
        {
            return size;
        }

        // Get the current capacity of the vector
        size_t getCapacity() const
        {
            return capacity;
        }

        // Add an element to the end of the vector
        void push_back(const T& value)
        {
            if (size == capacity)
            {
                resize();
            }
            array[size++] = value;
        }

        // Remove the last element of the vector
        void pop_back()
        {
            if (size == 0)
            {
                return;
            }
            --size;
        }

        // Erase an element at a specified position
        void erase(size_t index)
        {
            if (index >= size)
            {
                return; // Index out of bounds
            }

            // Shift elements to the left
            for (size_t i = index; i < size - 1; ++i)
            {
                array[i] = array[i + 1];
            }
            --size;
        }

        // Access element by index
        T& operator[](size_t index)
        {
            return array[index];
        }

        // Access element by index (const version)
        const T& operator[](size_t index) const
        {
            return array[index];
        }

        // Iterator class
        class iterator
        {
        private:
            T* ptr;

        public:
            iterator(T* ptr) : ptr(ptr) {}

            iterator& operator++()
            {
                ++ptr;
                return *this;
            }

            iterator operator++(int)
            {
                iterator temp = *this;
                ++(*this);
                return temp;
            }

            T& operator*() const
            {
                return *ptr;
            }

            T* operator->() const
            {
                return ptr;
            }

            bool operator==(const iterator& other) const
            {
                return ptr == other.ptr;
            }

            bool operator!=(const iterator& other) const
            {
                return ptr != other.ptr;
            }
        };

        // Begin iterator
        iterator begin()
        {
            return iterator(array);
        }

        // End iterator
        iterator end()
        {
            return iterator(array + size);
        }

        bool empty() const
        {
            return size == 0;
        }
    };
}
