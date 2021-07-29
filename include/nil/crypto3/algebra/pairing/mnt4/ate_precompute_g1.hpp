//---------------------------------------------------------------------------//
// Copyright (c) 2020-2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020-2021 Nikita Kaskov <nbering@nil.foundation>
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

#ifndef CRYPTO3_ALGEBRA_PAIRING_MNT4_298_ATE_PRECOMPUTE_G1_HPP
#define CRYPTO3_ALGEBRA_PAIRING_MNT4_298_ATE_PRECOMPUTE_G1_HPP

#include <nil/crypto3/algebra/curves/mnt4.hpp>
#include <nil/crypto3/algebra/pairing/detail/mnt4/298/types.hpp>

namespace nil {
    namespace crypto3 {
        namespace algebra {
            namespace pairing {

                template<std::size_t Version = 298>
                class mnt4_ate_precompute_g1;

                template<>
                class mnt4_ate_precompute_g1<298> {
                    using curve_type = curves::mnt4<298>;
                    using types_policy = detail::types_policy<curve_type>;
                    using g1_type = typename curve_type::g1_type;

                    using g1_affine_type = typename curve_type::g1_type<curves::coordinates::affine>;
                public:

                    using g1_precomputed_type = typename types_policy::ate_g1_precomputed_type;

                    static g1_precomputed_type process(
                        const typename g1_type::value_type &P) {

                        typename g1_affine_type::value_type Pcopy = P.to_affine();

                        g1_precomputed_type result;
                        result.PX = Pcopy.X;
                        result.PY = Pcopy.Y;
                        result.PX_twist = Pcopy.X * g2::twist;
                        result.PY_twist = Pcopy.Y * g2::twist;

                        return result;
                    }
                };
            }        // namespace pairing
        }            // namespace algebra
    }                // namespace crypto3
}    // namespace nil
#endif    // CRYPTO3_ALGEBRA_PAIRING_MNT4_298_ATE_PRECOMPUTE_G1_HPP
