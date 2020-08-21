//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef ALGEBRA_CURVES_NIST_P521_HPP
#define ALGEBRA_CURVES_NIST_P521_HPP

#include <nil/crypto3/pubkey/ec_group/curve_nist.hpp>
#include <nil/crypto3/algebra/curves/detail/element/p521.hpp>

#include <nil/algebra/detail/mp_def.hpp>

namespace nil {
    namespace algebra {
        namespace curves {

            /**
             * The NIST P-521 curve
             */
            template<std::size_t WordBits = limb_bits>
            struct p521 : public curve_nist_policy<521, WordBits> {
                typedef typename curve_nist_policy<521>::number_type number_type;

                constexpr static const number_type p =
                    0x1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_cppui521;

            };
        }    // namespace curves
    }        // namespace crypto3
}    // namespace nil

#endif    // ALGEBRA_CURVES_NIST_P521_HPP
