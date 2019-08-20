#pragma once
#include <atomic>
#include "BitwiseUtility.h"
#include "MathUtility.h"
inline namespace DebugDescriptor
{
        enum class queue_acquire
        {
                Null,
                Popped,
                Stolen
        };
        enum class init_bool
        {
                Null,
                True,
                False,
        };
        struct _GlobalDescriptor
        {
                queue_acquire _popped_or_stolen = queue_acquire::Null;
                long          _m_jobs_index     = 13131L;
                long          _t_old            = 13131L;
                long          _t_new            = 13131L;
                long          _b_old            = 13131L;
                long          _b_new            = 13131L;
                init_bool     _succeeded_cas    = init_bool::Null;
                init_bool     _empty_queue      = init_bool::Null;
                unsigned      _thread_index     = 1313131313U;

        } inline thread_local GlobalDescriptor;
		struct _GlobalAllocDescriptor
		{
                unsigned _allocation_offset = 13131L;
                unsigned _allocation_index  = 13131L;
		} inline thread_local GlobalAllocDescriptor;

} // namespace DebugDescriptor


inline namespace ContainerUtility
{

        //constexpr size_t divideRoundUp(size_t dividend, size_t divisor)
        //{
        //        return (dividend + divisor - 1) / divisor;
        //}

        struct VectorTLS
        {};

        struct TypelessVectorView
        {
                char* Begin;
                char* End;
                TypelessVectorView()
                {
                        Begin = 0;
                        End   = 0;
                }
                TypelessVectorView(void* Begin, void* End) : Begin((char*)Begin), End((char*)End)
                {}
                char& operator[](unsigned i)
                {
                        return *(Begin + (i));
                }
                const char& operator[](unsigned i) const
                {
                        return *(Begin + (i));
                }

                struct iterator
                {
                        char* curr;
                        iterator(char* curr)
                        {
                                this->curr = curr;
                        }
                        bool operator!=(const iterator& other)
                        {
                                return other.curr != this->curr;
                        }
                        iterator operator++()
                        {
                                char* temp = curr;
                                curr       = curr;
                                return iterator(temp);
                        }
                        char& operator*()
                        {
                                return *curr;
                        }
                };
                iterator begin()
                {
                        return iterator(Begin);
                }
                iterator end()
                {
                        return iterator(End);
                }
        };

        template <typename T>
        struct VectorImpl
        {
                T*     Begin;
                T*     End;
                size_t Capacity;
                size_t Alignment;
                struct iterator
                {
                        VectorImpl<T>& owner;
                        T*             curr;
                        iterator(VectorImpl<T>& owner, T* curr) : owner(owner)
                        {
                                this->curr = curr;
                        }
                        bool operator!=(const iterator& other)
                        {
                                return other.curr != this->curr;
                        }
                        iterator operator++()
                        {
                                T* temp = curr;
                                curr    = (T*)(((char*)curr) + owner.Alignment);
                                return iterator(owner, temp);
                        }
                        T& operator*()
                        {
                                return *curr;
                        }
                };
                VectorImpl()
                {
                        this->Begin     = 0;
                        this->End       = 0;
                        this->Capacity  = 0;
                        this->Alignment = 0;
                }
                VectorImpl(T* begin, T* end, size_t capacity, size_t alignment)
                {
                        this->Begin     = begin;
                        this->End       = end;
                        this->Capacity  = capacity;
                        this->Alignment = alignment;
                }
                VectorImpl(VectorImpl&& other)
                {
                        this->Begin        = other.Begin;
                        this->End          = other.End;
                        this->Capacity     = other.Capacity;
                        this->Alignment    = other.Alignment;
                        other.Begin        = 0;
                        other.End          = 0;
                        other.Capacity     = 0;
                        other.BitAlignment = 0;
                }
                T& operator[](unsigned i)
                {
                        return *(T*)((char*)Begin + i * Alignment);
                }
                const T& operator[](unsigned i) const
                {
                        return *(const T*)((char*)Begin + i * Alignment);
                }
                void push_back(const T& element)
                {
                        assert(End != Begin + Capacity);
                        *End = element;
                        ((char*&)End) += Alignment;
                }
                void pop_back()
                {
                        assert(End != Begin);
                        ((char*&)End) -= Alignment;
                }

                iterator begin()
                {
                        return iterator(*this, Begin);
                }
                iterator end()
                {
                        return iterator(*this, End);
                }
        };

        template <typename T, size_t N = 32, size_t Alignment = 1>
        struct StackVector : VectorImpl<T>
        {
                static constexpr unsigned Alignment = GetAlignment(sizeof(T), Alignment);

                char Buffer[N * Alignment];
                StackVector() : VectorImpl<T>((T*)Buffer, (T*)Buffer, N, Alignment)
                {}
        };

        constexpr unsigned GetAlignment(unsigned ElementSize, unsigned RequestedAlignment)
        {
                if (ElementSize > RequestedAlignment && !(ElementSize % RequestedAlignment))
                        return ElementSize;
                if (ElementSize > RequestedAlignment && (ElementSize % RequestedAlignment))
                        return GetAlignment(ElementSize, nextPowerOf2(RequestedAlignment + 1));
                if (ElementSize < RequestedAlignment)
                        return RequestedAlignment;
        }

        template <typename T, unsigned Alignment = 1>
        struct AllocVector : public VectorImpl<T>
        {
                static constexpr unsigned Alignment = GetAlignment(sizeof(T), Alignment);
                static constexpr unsigned Padding   = Alignment - sizeof(T);
                AllocVector() : VectorImpl<T>(0, 0, 0, Alignment)
                {}
                AllocVector(unsigned sz) : VectorImpl<T>((T*)malloc(Alignment * sz), 0, sz, Alignment)
                {
                        (char*&)VectorImpl<T>::End = (char*)VectorImpl<T>::Begin + sz * Alignment;
                }
                void Realloc(unsigned sz)
                {
                        VectorImpl<T>::Begin = (T*)realloc(VectorImpl<T>::Begin, sz * Alignment);
                        VectorImpl<T>::End   = VectorImpl<T>::Begin + sz * Alignment;
                }
                AllocVector(AllocVector&& other)
                {
                        if (&other == this)
                                return;
                        _aligned_free(VectorImpl<T>::Begin);
                        VectorImpl<T>::Alignment = other.Alignment;
                        VectorImpl<T>::Capacity  = other.Capacity;
                        VectorImpl<T>::Begin     = other.Begin;
                        VectorImpl<T>::End       = other.End;
                        other.Alignment          = 0;
                        other.Capacity           = 0;
                        other.Begin              = 0;
                        other.End                = 0;
                }
                AllocVector(const AllocVector& other)
                {
                        if (&other == this)
                                return;
                        for (unsigned i = 0; i < other.Capacity && i < VectorImpl<T>::Capacity; i++)
                        {
                                VectorImpl<T>::operator[](i) = other[i];
                        }
                }
                AllocVector& operator=(AllocVector&& other) noexcept
                {
                        if (&other == this)
                                return *this;
                        free(VectorImpl<T>::Begin);
                        VectorImpl<T>::Alignment = other.Alignment;
                        VectorImpl<T>::Capacity  = other.Capacity;
                        VectorImpl<T>::Begin     = other.Begin;
                        VectorImpl<T>::End       = other.End;
                        other.Capacity           = 0;
                        other.Begin              = 0;
                        other.End                = 0;
                        return *this;
                }
                void resize(unsigned sz)
                {
                        if (sz > VectorImpl<T>::Capacity)
                        {
                                AllocVector<T, Alignment> other = AllocVector<T, Alignment>(sz);
                                size_t difference               = (size_t)VectorImpl<T>::End - (size_t)VectorImpl<T>::Begin;
                                for (unsigned i = 0; i < sz && i < difference / Alignment; i++)
                                        other[i] = VectorImpl<T>::operator[](i);
                                *this = std::move(other);
                        }
                        else
                        {
                                VectorImpl<T>::Capacity = sz;
                                if ((size_t)VectorImpl<T>::End - (size_t)VectorImpl<T>::Begin > VectorImpl<T>::Capacity)
                                {
                                        (char*&)VectorImpl<T>::End =
                                            (char*)VectorImpl<T>::Begin + VectorImpl<T>::Capacity * Alignment;
                                }
                        }
                }
                ~AllocVector()
                {
                        free((void*)VectorImpl<T>::Begin);
                }
                void Free()
                {
                        free((void*)VectorImpl<T>::Begin);
                        VectorImpl<T>::Begin = 0;
                }
        };

        template <typename T, unsigned MaxElements>
        struct RingBuffer
        {
                static constexpr unsigned MaxElements      = nextPowerOf2(MaxElements);
                static constexpr unsigned Mask             = MaxElements - 1;
                unsigned                  AllocationOffset = 0;
                T*                        Buffer;
                RingBuffer()
                {
                        Buffer = static_cast<T*>(malloc(MaxElements * sizeof(T)));
                }
                ~RingBuffer()
                {
                        free(Buffer);
                }
                T* Allocate()
                {
                        T* temp                             = (Buffer + (AllocationOffset & Mask));
                        AllocationOffset++;
                        return temp;
                }
        };

        template <typename T, unsigned Alignment, unsigned MaxElements>
        struct RingBufferAtomic
        {
                static constexpr unsigned MaxElements = nextPowerOf2(MaxElements);
                static constexpr unsigned Mask        = MaxElements - 1;
                static constexpr unsigned Alignment   = GetAlignment(sizeof(T), Alignment);

                char* const           Buffer;
                std::atomic<unsigned> previousAllocationOffset = 0 - 1;
                RingBufferAtomic() : Buffer(static_cast<char*>(_aligned_malloc(MaxElements * Alignment, Alignment)))
                {}
                ~RingBufferAtomic()
                {
                        _aligned_free(reinterpret_cast<void*>(const_cast<char*>(Buffer)));
                }
                T* Allocate()
                {
                        const unsigned allocationOffset = previousAllocationOffset++;
                        return reinterpret_cast<T*>(Buffer + ((allocationOffset & Mask) * Alignment));
                }
        };

}; // namespace ContainerUtility