//---------------------------------------------------------------------------//
// Copyright (c) 2020 Alexander Sokolov <asokolov@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_SHA3_PADDING_HPP
#define CRYPTO3_SHA3_PADDING_HPP

#include <nil/crypto3/hash/detail/sha3/sha3_policy.hpp>
#include <nil/crypto3/detail/unbounded_shift.hpp>

namespace nil {
    namespace crypto3 {
        namespace hash {
            namespace detail {
                template<typename Hash>
                class sha3_padding {
                    typedef Hash policy_type;

                    typedef typename policy_type::digest_endian endian_type;

                    constexpr static const std::size_t word_bits = policy_type::word_bits;
                    typedef typename policy_type::word_type word_type;

                    constexpr static const std::size_t state_bits = policy_type::state_bits;
                    constexpr static const std::size_t state_words = policy_type::state_words;
                    typedef typename policy_type::state_type state_type;

                    constexpr static const std::size_t block_bits = policy_type::block_bits;
                    constexpr static const std::size_t block_words = policy_type::block_words;
                    typedef typename policy_type::block_type block_type;

                    constexpr static const std::size_t digest_bits = policy_type::digest_bits;
                    typedef typename policy_type::digest_type digest_type;

                    typedef ::nil::crypto3::detail::injector<stream_endian::big_octet_little_bit, 
                        word_bits, block_words, block_bits> injector_type;

                    bool is_last;

                public:
                    sha3_padding() : is_last(true) {
                    }

                    bool is_last_block() const {
                        return is_last;
                    }

                    void operator()(block_type &block, std::size_t &block_seen) {
                        using namespace nil::crypto3::detail;

                        if ((block_bits - block_seen) > 3) {
                            // pad 011
                            injector_type::inject(high_bits<word_type, word_bits>(~word_type(), 2) >> 5, 3, block, block_seen);
                            // pad 0*
                            block_type zeros;
                            std::fill(zeros.begin(), zeros.end(), 0);
                            injector_type::inject(zeros, block_bits - 1 - block_seen, block, block_seen);
                            // pad 1
                            injector_type::inject(high_bits<word_type, word_bits>(~word_type(), 1) >> 7, 1, block, block_seen);
                        }

                        else {
                            is_last = false;
                            std::size_t ind = block_bits - block_seen - 1;
                            block[block_words - 1] &= ~high_bits<word_type, word_bits>(~word_type(), ind + 1);
                            block[block_words - 1] |= high_bits<word_type, word_bits>(~word_type(), ind);
                        }
                    }

                    void process_last(block_type &block, std::size_t &block_seen) {
                        using namespace nil::crypto3::detail;

                        std::size_t ind = 3 - (block_bits - block_seen);
                        // Insert remaining padding bits
                        injector_type::inject(low_bits<word_type, word_bits>(~word_type(), ind), ind, block, block_seen);
                        // pad 0*
                        block_type zeros;
                        std::fill(zeros.begin(), zeros.end(), 0);
                        injector_type::inject(zeros, block_bits - 1 - ind, block, block_seen);
                        // pad 1
                        injector_type::inject(high_bits<word_type, word_bits>(~word_type(), 1) >> 7, 1, block, block_seen);
                    }
                };
            }    // namespace detail
        }        // namespace hash
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_SHA3_PADDING_HPP
