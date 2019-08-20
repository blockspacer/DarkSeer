#pragma once
#include "ContainerUtility.h"
inline namespace OffsetIndexSequenceUtility
{
        enum class SFINAEDummyArgType
        {
        };

        template <typename... Ts>
        struct TypePack
        {};

        template <unsigned... Is, unsigned... Js>
        constexpr auto concat_integer_sequence(std::index_sequence<Is...>, std::index_sequence<Js...>)
        {
                return std::index_sequence<Is..., Js...>{};
        }

        template <unsigned Prev, typename Seq, typename T>
        constexpr auto size_accum_impl(SFINAEDummyArgType)
        {
                return concat_integer_sequence(Seq{}, std::index_sequence<Prev>{});
        }
        template <unsigned Prev, typename Seq, typename T, typename... Ts>
        constexpr auto size_accum_impl(typename std::enable_if<sizeof...(Ts), SFINAEDummyArgType>::type)
        {
                auto new_seq = concat_integer_sequence(Seq{}, std::index_sequence<Prev>{});
                return size_accum_impl<sizeof(T) + Prev, decltype(new_seq), Ts...>(SFINAEDummyArgType{});
        }
        template <typename T, typename... Ts>
        constexpr auto size_accum_init(typename std::enable_if<sizeof...(Ts), SFINAEDummyArgType>::type)
        {
                return size_accum_impl<sizeof(T), decltype(std::index_sequence<0>{}), Ts...>(SFINAEDummyArgType{});
        }
        template <typename T>
        constexpr auto size_accum_init(SFINAEDummyArgType)
        {
                return std::index_sequence<0>{};
        }
        template <typename... Ts>
        constexpr auto accum_byte_offset_integer_sequence()
        {
                return size_accum_init<Ts...>(SFINAEDummyArgType{});
        }
        template <typename... Ts>
        using BitfieldSliceOffsetIndexSequenceHelper = decltype(accum_byte_offset_integer_sequence<Ts...>());
} // namespace OffsetIndexSequenceUtility

inline namespace SizeOfPackUtility
{
        struct ptr_type
        {
                static constexpr void* null_ref{};
        };
        template <unsigned Sz>
        struct integral_type
        {
                using type = void;
        };
        template <>
        struct integral_type<8>
        {
                using type = uint8_t;
                static constexpr type null_ref{};
        };
        template <>
        struct integral_type<16>
        {
                using type = uint16_t;
                static constexpr type null_ref{};
        };
        template <>
        struct integral_type<32>
        {
                using type = uint32_t;
                static constexpr type null_ref{};
        };
        template <>
        struct integral_type<64>
        {
                using type = uint64_t;
                static constexpr type null_ref{};
        };

        template <typename T, typename... Ts>
        struct SpliceFunctionType
        {
                using ReturnType = T;
                using ParamTypes = std::tuple<Ts...>;
                SpliceFunctionType(T (*)(Ts...))
                {}
        };
        template <typename Tuple, typename OtherTuple, unsigned N>
        constexpr typename std::enable_if<!N, bool>::type are_convertible_impl()
        {
                return std::is_convertible<std::tuple_element<N, Tuple>, std::tuple_element<N, OtherTuple>>::value;
        }
        template <typename Tuple, typename OtherTuple, unsigned N>
        constexpr typename std::enable_if<N, bool>::type are_convertible_impl()
        {
                if (!std::is_convertible<std::tuple_element<N, Tuple>, std::tuple_element<N, OtherTuple>>::value)
                        return false;
                return are_convertible_impl<Tuple, OtherTuple, N - 1>();
        }
        template <typename Tuple, typename OtherTuple>
        constexpr typename std::enable_if<std::tuple_size<Tuple>::value, bool>::type are_convertible()
        {
                if (std::tuple_size<Tuple>::value != std::tuple_size<OtherTuple>::value)
                        return false;
                return are_convertible_impl<Tuple, OtherTuple, std::tuple_size<Tuple>::value - 1>();
        }
        template <auto F, typename... OtherArgs>
        constexpr bool args_are_convertible()
        {
                return are_convertible<typename decltype(SpliceFunctionType(F))::ParamTypes, std::tuple<OtherArgs...>>();
        }

        template <typename T>
        constexpr size_t SizeOfPackedTsImpl(size_t size)
        {
                return sizeof(T) + size;
        }
        template <typename T, typename... Ts>
        constexpr typename std::enable_if<sizeof...(Ts), size_t>::type SizeOfPackedTsImpl(size_t size)
        {
                return SizeOfPackedTsImpl<Ts...>(sizeof(T) + size);
        }
        template <typename T, typename... Ts>
        constexpr typename std::enable_if<sizeof...(Ts), size_t>::type SizeOfPackedTs()
        {
                return SizeOfPackedTsImpl<Ts...>(sizeof(T));
        }
        template <typename T>
        constexpr size_t SizeOfPackedTs()
        {
                return sizeof(T);
        }
} // namespace SizeOfPackUtility

inline namespace PackTinyUtility
{
        template <typename T>
        void PackArgs(char* _dst, T t)
        {
                memcpy(const_cast<char*>(_dst), &t, sizeof(T));
        }
        template <typename T, typename... Ts>
        typename std::enable_if<sizeof...(Ts)>::type PackArgs(char* _dst, T t, Ts... ts)
        {
                memcpy(const_cast<char*>(_dst), &t, sizeof(T));
                return PackArgs(_dst + sizeof(T), ts...);
        }
        template <typename T, typename... TupleTs>
        std::tuple<TupleTs..., T> UnpackArgsInternal(char* _src, std::tuple<TupleTs...> tuple)
        {
                std::tuple<T> unpackedArgs = *(T*)_src;
                return std::tuple_cat(tuple, unpackedArgs);
        }
        template <typename T, typename... Ts, typename... TupleTs>
        typename std::enable_if<sizeof...(Ts), std::tuple<TupleTs..., T, Ts...>>::type UnpackArgsInternal(
            char*                  _src,
            std::tuple<TupleTs...> tuple)
        {
                std::tuple<T>             unpackedArg  = *(T*)_src;
                std::tuple<TupleTs..., T> unpackedArgs = std::tuple_cat(tuple, unpackedArg);
                return UnpackArgsInternal<Ts...>(_src + sizeof(T), unpackedArgs);
        }
        template <typename T, typename... Ts>
        typename std::enable_if<sizeof...(Ts), std::tuple<T, Ts...>>::type UnpackArgs(char* _src)
        {
                std::tuple<T> unpackedArg(*(T*)_src);
                return UnpackArgsInternal<Ts...>(_src + sizeof(T), unpackedArg);
        }
        template <typename T>
        std::tuple<T> UnpackArgs(char* _src)
        {
                return std::tuple<T>(*(T*)_src);
        }
} // namespace PackTinyUtility

inline namespace BitFieldPackUtility
{
        template <typename bitfield_base_type,
                  bitfield_base_type Sz,
                  bitfield_base_type BitAlignment,
                  bitfield_base_type Offset>
        struct bitfield_member
        {
                static bitfield_base_type constexpr _bitmask = ((std::integral_constant<bitfield_base_type, 1>::value << Sz) -
                                                                std ::integral_constant<bitfield_base_type, 1>::value)
                                                               << Offset;
                static constexpr bitfield_base_type offset = Offset;
                bitfield_base_type&                 value;
                // Note: only use dfault ctor for use with decltype
                bitfield_member() : value((bitfield_base_type&)integral_type<BitAlignment>::null_ref)
                {}
                bitfield_member(bitfield_base_type& value) : value(value)
                {}
                bitfield_member(bitfield_member&& other) noexcept : value(other.value)
                {}
                bitfield_base_type Get()
                {
                        return (value << Offset) & ((std::integral_constant<bitfield_base_type, 1>::value << Sz) -
                                                    std::integral_constant<bitfield_base_type, 1>::value);
                }
                void Set(bitfield_base_type input_value)
                {
                        value = (value & ~_bitmask) | (((input_value << Offset) & _bitmask));
                }
        };

        template <unsigned BitAlignment, unsigned... Is, typename... Ts>
        constexpr auto create_bitfield_tuple_type(std::index_sequence<Is...>, TypePack<Ts...>)
        {
                static_assert(SizeOfPackedTs<Ts...>() <= BitAlignment,
                              "Bitfield vector list is too large for the Alignment specified");
                return std::tuple<
                    bitfield_member<integral_type<BitAlignment>::type, sizeof(Ts) * 8, BitAlignment, Is * 8>...>();
        }
        template <unsigned BitAlignment, typename SequenceType, typename PackType>
        using bitfield_tuple_type = decltype(create_bitfield_tuple_type<BitAlignment>(SequenceType{}, PackType{}));

        template <unsigned BitAlignment, typename... Ts>
        struct Bitfield
        {
                using BitfieldSliceOffsetIndexSequence = BitfieldSliceOffsetIndexSequenceHelper<Ts...>;
                using bitfield_tuple_type =
                    decltype(create_bitfield_tuple_type<BitAlignment>(BitfieldSliceOffsetIndexSequence{}, TypePack<Ts...>{}));
                using bitfield_base_type = typename integral_type<BitAlignment>::type;

                bitfield_base_type& bitfield_base_value;

                Bitfield() : bitfield_base_value(integral_type<BitAlignment>::null_ref)
                {}
                Bitfield(bitfield_base_type& bitfield_base_value) : bitfield_base_value(bitfield_base_value)
                {}

                bitfield_tuple_type GetBitfieldMembers()
                {
                        return GetBitfieldMembersImpl(bitfield_tuple_type{});
                }

            private:
                template <typename... bitfield_member_types>
                bitfield_tuple_type GetBitfieldMembersImpl(std::tuple<bitfield_member_types...>)
                {
                        return std::tuple(bitfield_member_types{bitfield_base_value}...);
                }

            public:
                template <std::size_t N>
                auto get()
                {
                        return std::get<N>((bitfield_tuple_type)GetBitfieldMembers());
                }
        };
        template <unsigned BitAlignment, typename... Ts>
        struct std::tuple_size<Bitfield<BitAlignment, Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)>
        {};

        template <std::size_t N, unsigned BitAlignment, typename... Ts>
        struct std::tuple_element<N, BitFieldPackUtility::Bitfield<BitAlignment, Ts...>>
        {
                using type = decltype(std::declval<BitFieldPackUtility::Bitfield<BitAlignment, Ts...>>().get<N>());
        };


}; // namespace BitFieldPackUtility

inline namespace BitfieldVectorUtility
{
        template <typename T, unsigned BitAlignment = 1, unsigned Offset = 0>
        struct BitfieldSliceVector
        {
                char* const Begin;
                char* const End;
                BitfieldSliceVector() : Begin((char* const)ptr_type::null_ref), End((char* const)ptr_type::null_ref)
                {}
                BitfieldSliceVector(char* const Begin, char* const End) : Begin(Begin), End(End)
                {}

                BitfieldSliceVector(VectorImpl<T> managedVector) :
                    Begin((char* const&)managedVector.Begin),
                    End((char* const&)managedVector.End)
                {}
                T& operator[](unsigned i)
                {
                        return *(T*)(Begin + (i * BitAlignment) + Offset);
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
                                curr       = curr + BitAlignment;
                                return iterator(temp);
                        }
                        T& operator*()
                        {
                                return *(T*)(curr + Offset);
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

        template <unsigned BitAlignment, unsigned... Offsets, typename... Ts>
        constexpr auto BitfieldVectorHelperImpl(std::index_sequence<Offsets...>, TypePack<Ts...>)
        {
                static_assert(SizeOfPackedTs<Ts...>() <= BitAlignment,
                              "A Bitfield tuple's argument list is too large for its alignment (alignment = total capacity for "
                              "a single data slice)");
                return std::tuple<BitfieldSliceVector<Ts, BitAlignment, Offsets>...>();
        }
        template <unsigned BitAlignment, typename SequenceType, typename PackType>
        using BitfieldVectorHelper = decltype(BitfieldVectorHelperImpl<BitAlignment>(SequenceType{}, PackType{}));

        // TODO allocation is subtly called int the constructor with size_t argument and free is left to the user.
        // TODO iterators
        // TODO hide TypelessVectorView implementation / overload
        template <unsigned BitAlignment, typename... Ts>
        struct PackedTupleVector : protected TypelessVectorView
        {
                static_assert(SizeOfPackedTs<Ts...>() * 8 <= BitAlignment, "argument size too large");
                using BitfieldSliceOffsetIndexSequence = BitfieldSliceOffsetIndexSequenceHelper<Ts...>;
                using BitfieldVector   = BitfieldVectorHelper<BitAlignment, BitfieldSliceOffsetIndexSequence, TypePack<Ts...>>;
                using TupleType        = std::tuple<Ts...>;
                using BitfieldBaseType = typename Bitfield<BitAlignment, Ts...>::bitfield_base_type;
                static constexpr unsigned FreeBits = BitAlignment - SizeOfPackedTs<Ts...>() * 8;
                char                      Padding[FreeBits / 8];
                PackedTupleVector(size_t sz)
                {
                        TypelessVectorView::Begin = (char*)_aligned_malloc(sz * BitAlignment, BitAlignment);
                        TypelessVectorView::End   = TypelessVectorView::Begin + (sz * BitAlignment);
                }
                PackedTupleVector(void* Begin, void* End) : TypelessVectorView(Begin, End)
                {}

                BitfieldVector GetBitfieldVector()
                {
                        return GetBitfieldVectorImpl(BitfieldSliceOffsetIndexSequence{});
                }
                Bitfield<BitAlignment, Ts...> GetBitField(unsigned i)
                {
                        return Bitfield<BitAlignment, Ts...>(
                            *(BitfieldBaseType*)std::addressof(TypelessVectorView::operator[](i)));
                }

            private:
                template <unsigned... Offsets>
                BitfieldVector GetBitfieldVectorImpl(std::index_sequence<Offsets...>)
                {
                        return BitfieldVector(BitfieldSliceVector<Ts, BitAlignment, Offsets>{
                            (char* const)TypelessVectorView::Begin, (char* const)TypelessVectorView::End}...);
                }
        };
}; // namespace BitfieldVectorUtility