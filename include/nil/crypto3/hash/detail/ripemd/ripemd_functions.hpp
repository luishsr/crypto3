//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_RIPEMD_FUNCTIONS_HPP
#define CRYPTO3_RIPEMD_FUNCTIONS_HPP

#include <nil/crypto3/detail/basic_functions.hpp>

namespace nil {
    namespace crypto3 {
        namespace hash {
            namespace detail {
                template<std::size_t WordBits>
                struct ripemd_functions : public ::nil::crypto3::detail::basic_functions<WordBits> {
                    typedef ::nil::crypto3::detail::basic_functions<WordBits> policy_type;
                    
                    constexpr static const std::size_t word_bits = policy_type::word_bits;
                    typedef typename policy_type::word_type word_type;

                    struct f1 {
                        inline word_type operator()(word_type x, word_type y, word_type z) const {
                            return x ^ y ^ z;
                        }
                    };

                    struct f2 {
                        inline word_type operator()(word_type x, word_type y, word_type z) const {
                            return (x & y) | (~x & z);
                        }
                    };

                    struct f3 {
                        inline word_type operator()(word_type x, word_type y, word_type z) const {
                            return (x | ~y) ^ z;
                        }
                    };

                    struct f4 {
                        inline word_type operator()(word_type x, word_type y, word_type z) const {
                            return (x & z) | (y & ~z);
                        }
                    };

                    struct f5 {
                        inline word_type operator()(word_type x, word_type y, word_type z) const {
                            return x ^ (y | ~z);
                        }
                    };

                    template<typename F>
                    inline static void transform(word_type &a, word_type &b, word_type &c, word_type &d, word_type x,
                                                 word_type k, word_type s) {
                        word_type T = policy_type::rotl(a + F()(b, c, d) + x + k, s);
                        a = d;
                        d = c;
                        c = b;
                        b = T;
                    }

                    template<typename Functor>
                    inline static void transform(word_type &a, word_type &b, word_type &c, word_type &d, word_type &e,
                                                 word_type x, word_type k, word_type s) {
                        word_type T = policy_type::rotl(a + Functor()(b, c, d) + x + k, s) + e;
                        a = e;
                        e = d;
                        d = policy_type::template rotl<10>(c);
                        c = b;
                        b = T;
                    }
                };
            }    // namespace detail
        }        // namespace hash
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_RIPEMD_FUNCTIONS_HPP
