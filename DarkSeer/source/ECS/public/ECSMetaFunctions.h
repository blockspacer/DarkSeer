#pragma once
template <class B, class = void>
struct requires_constructor : std::false_type
{};
template <class B>
struct requires_constructor<B, std::void_t<typename B::requires_constructor_tag>> : std::true_type
{};