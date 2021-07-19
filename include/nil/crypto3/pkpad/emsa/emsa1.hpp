//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Ilias Khairullin <ilias@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_PUBKEY_PADDING_EMSA1_HPP
#define CRYPTO3_PUBKEY_PADDING_EMSA1_HPP

#include <iterator>
#include <type_traits>

#include <nil/crypto3/pkpad/emsa.hpp>

#include <nil/crypto3/algebra/type_traits.hpp>

#include <nil/crypto3/hash/algorithm/hash.hpp>

#include <nil/marshalling/field_type.hpp>
#include <nil/crypto3/marshalling/types/algebra/field_element.hpp>

namespace nil {
    namespace crypto3 {
        namespace pubkey {
            namespace padding {
                namespace detail {
                    template<typename MsgReprType, typename Hash, typename = void>
                    struct emsa1_encoding_policy;

                    template<typename MsgReprType, typename Hash>
                    struct emsa1_encoding_policy<
                        MsgReprType, Hash,
                        typename std::enable_if<
                            algebra::is_field<typename MsgReprType::field_type>::value &&
                            !algebra::is_extended_field<typename MsgReprType::field_type>::value>::type> {
                    protected:
                        typedef Hash hash_type;
                        typedef typename MsgReprType::field_type field_type;
                        typedef ::nil::crypto3::marshalling::types::field_element<
                            ::nil::marshalling::field_type<::nil::marshalling::option::big_endian>, field_type>
                            marshalling_field_element_type;

                    public:
                        typedef MsgReprType msg_repr_type;
                        typedef accumulator_set<hash_type> accumulator_type;
                        typedef msg_repr_type result_type;

                        template<typename InputRange>
                        static inline void update(accumulator_type &acc, const InputRange &range) {
                            hash<hash_type>(range, acc);
                        }

                        template<typename InputIterator>
                        static inline void update(accumulator_type &acc, InputIterator first, InputIterator last) {
                            hash<hash_type>(first, last, acc);
                        }

                        static inline result_type process(accumulator_type &acc) {
                            typename hash_type::digest_type digest = accumulators::extract::hash<hash_type>(acc);
                            marshalling_field_element_type marshalling_field_element;
                            // TODO: for what purpose we supply size if read() didn't use it inside
                            // TODO: (serialization) why we need to create another object for marshalling from original
                            //  object by copy it inside
                            // TODO: (deserialization) why marshalling object constructor take non-ref arg, for
                            //  multiprecision type it might cause overhead
                            marshalling_field_element.read(digest.cbegin(), digest.size());
                            return crypto3::marshalling::types::construct_field_element(marshalling_field_element);
                        }
                    };

                    template<typename MsgReprType, typename Hash, typename = void>
                    struct emsa1_verification_policy;

                    template<typename MsgReprType, typename Hash>
                    struct emsa1_verification_policy<
                        MsgReprType, Hash,
                        typename std::enable_if<
                            algebra::is_field<typename MsgReprType::field_type>::value &&
                            !algebra::is_extended_field<typename MsgReprType::field_type>::value>::type> {
                    protected:
                        typedef Hash hash_type;
                        typedef typename MsgReprType::field_type field_type;
                        typedef ::nil::crypto3::marshalling::types::field_element<
                            ::nil::marshalling::field_type<::nil::marshalling::option::big_endian>, field_type>
                            marshalling_field_element_type;
                        typedef emsa1_encoding_policy<MsgReprType, Hash> encoding_policy;

                    public:
                        typedef MsgReprType msg_repr_type;
                        typedef typename encoding_policy::accumulator_type accumulator_type;
                        typedef bool result_type;

                        template<typename InputRange>
                        static inline void update(accumulator_type &acc, const InputRange &range) {
                            encoding_policy::update(range, acc);
                        }

                        template<typename InputIterator>
                        static inline void update(accumulator_type &acc, InputIterator first, InputIterator last) {
                            encoding_policy::update(first, last, acc);
                        }

                        static inline result_type process(accumulator_type &acc, const msg_repr_type &msg_repr) {
                            return encoding_policy::process(acc) == msg_repr;
                        }
                    };
                }    // namespace detail

                /*!
                 * @brief EMSA1 from IEEE 1363.
                 * Essentially, sign the hash directly
                 *
                 * @tparam MsgReprType
                 * @tparam Hash
                 * @tparam l
                 */
                template<typename MsgReprType, typename Hash>
                struct emsa1 {
                    typedef MsgReprType msg_repr_type;
                    typedef Hash hash_type;

                    typedef detail::emsa1_encoding_policy<MsgReprType, Hash> encoding_policy;
                    typedef detail::emsa1_verification_policy<MsgReprType, Hash> verification_policy;
                };
            }    // namespace padding
        }        // namespace pubkey
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_PUBKEY_PADDING_EMSA1_HPP
