
#include <cstddef>
#include <initializer_list>

template<typename T> class array{
    private:
        size_t size_ = 0;
        size_t capacity_ = 0;
        T *arr_ = nullptr;

    public:
        array(size_t initialCap = 1){
            size_ = 0;
            capacity_ = (initialCap == 0) ? 1 : initialCap;
            arr_ = new T[capacity_]();
        }
        array(std::initializer_list<T> init)
            : size_(init.size()),
            capacity_(init.size()),
            arr_(new T[capacity_])
        {
            size_t i = 0;
            for (const auto& v : init) {
                arr_[i++] = v;
            }
        }
        array(const array&) = delete;
        array& operator=(const array&) = delete;

        ~array(){
            delete[] arr_;
        }

        size_t size() const{
            return size_;
        }

        size_t capacity() const{
            return capacity_;
        }

        T* data(){
            return arr_;
        };

        const T* data() const {
            return arr_;
        }

        void add(const T& element) {
            if (size_ == capacity_) {
                size_t newCap = (capacity_ == 0) ? 1 : capacity_ * 2;
                T* newArr = new T[newCap]();              // () wichtig

                for (size_t i = 0; i < size_; ++i)
                    newArr[i] = arr_[i];

                delete[] arr_;
                arr_ = newArr;
                capacity_ = newCap;
            }

            arr_[size_] = element;
            ++size_;
        }

        void resize(size_t n) {
            reserve(n);
            size_ = n;
        }

        void reserve(size_t n) {
            if (n <= capacity_) return;

            size_t newCap = capacity_ ? capacity_ : 1;
            while (newCap < n) newCap *= 2;

            T* newArr = new T[newCap]();
            for (size_t i = 0; i < size_; ++i) newArr[i] = arr_[i];

            delete[] arr_;
            arr_ = newArr;
            capacity_ = newCap;
        }

        bool empty(){
            if(size_ == 0){ return true;}
            else{ return false; }
        }

        array(array&& other) noexcept
            : size_(other.size_), capacity_(other.capacity_), arr_(other.arr_) {
            other.size_ = 0;
            other.capacity_ = 0;
            other.arr_ = nullptr;
        }

        array& operator=(array&& other) noexcept {
            if (this != &other) {
                delete[] arr_;
                size_ = other.size_;
                capacity_ = other.capacity_;
                arr_ = other.arr_;
                other.size_ = 0;
                other.capacity_ = 0;
                other.arr_ = nullptr;
            }
            return *this;
        }
        T& operator[](size_t i){ return arr_[i];}
        T* begin() { return arr_; }
        T* end() { return arr_ + size_; }

        const T& operator[](size_t i) const {return arr_[i];}
        const T* begin() const { return arr_; }
        const T* end() const { return arr_ + size_; }

};