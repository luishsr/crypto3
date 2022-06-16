//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
// Copyright (c) 2022 Aleksei Moskvin <alalmoskvin@nil.foundation>
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

#ifndef CRYPTO3_ZK_BATCHED_LIST_POLYNOMIAL_COMMITMENT_SCHEME_HPP
#define CRYPTO3_ZK_BATCHED_LIST_POLYNOMIAL_COMMITMENT_SCHEME_HPP

#include <nil/crypto3/math/polynomial/polynomial.hpp>
#include <nil/crypto3/math/polynomial/lagrange_interpolation.hpp>

#include <nil/crypto3/container/merkle/tree.hpp>
#include <nil/crypto3/container/merkle/proof.hpp>

#include <nil/crypto3/zk/transcript/fiat_shamir.hpp>
#include <nil/crypto3/zk/commitments/detail/polynomial/basic_fri.hpp>
#include <nil/crypto3/zk/commitments/detail/polynomial/basic_batched_fri.hpp>
#include <nil/crypto3/zk/commitments/polynomial/lpc.hpp>
#include <nil/crypto3/zk/commitments/polynomial/fri.hpp>
#include <nil/crypto3/zk/commitments/polynomial/batched_fri.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace commitments {

                /**
                 * @brief Based on the FRI Commitment description from \[ResShift].
                 * @tparam d ...
                 * @tparam Rounds Denoted by r in \[Placeholder].
                 *
                 * References:
                 * \[Placeholder]:
                 * "PLACEHOLDER: Transparent SNARKs from List
                 * Polynomial Commitment IOPs",
                 * Assimakis Kattis, Konstantin Panarin, Alexander Vlasov,
                 * Matter Labs,
                 * <https://eprint.iacr.org/2019/1400.pdf>
                 */
                //                template<typename FieldType, typename LPCParams, std::size_t BatchSize = 0>
                //                struct batched_list_polynomial_commitment;

                template<typename FieldType, typename LPCParams, std::size_t BatchSize>
                struct batched_list_polynomial_commitment
                    : public detail::basic_batched_fri<FieldType,
                                                       typename LPCParams::merkle_hash_type,
                                                       typename LPCParams::transcript_hash_type,
                                                       LPCParams::m> {

                    using merkle_hash_type = typename LPCParams::merkle_hash_type;

                    constexpr static const std::size_t lambda = LPCParams::lambda;
                    constexpr static const std::size_t r = LPCParams::r;
                    constexpr static const std::size_t m = LPCParams::m;
                    constexpr static const std::size_t leaf_size = BatchSize;

                    typedef LPCParams lpc_params;

                    typedef typename containers::merkle_proof<merkle_hash_type, 2> merkle_proof_type;

                    using basic_fri = detail::basic_batched_fri<FieldType,
                                                                typename LPCParams::merkle_hash_type,
                                                                typename LPCParams::transcript_hash_type,
                                                                m>;

                    using precommitment_type = typename basic_fri::precommitment_type;
                    using commitment_type = typename basic_fri::commitment_type;

                    struct proof_type {
                        bool operator==(const proof_type &rhs) const {
                            return z == rhs.z && fri_proof == rhs.fri_proof && T_root == rhs.T_root;
                        }
                        bool operator!=(const proof_type &rhs) const {
                            return !(rhs == *this);
                        }

                        typename std::conditional<leaf_size != 0,
                                                  std::array<std::vector<typename FieldType::value_type>, leaf_size>,
                                                  std::vector<std::vector<typename FieldType::value_type>>>::type z;

                        commitment_type T_root;

                        std::array<typename basic_fri::proof_type, lambda> fri_proof;
                    };
                };

                template<typename FieldType, typename LPCParams, std::size_t BatchSize>
                using batched_lpc = batched_list_polynomial_commitment<FieldType, LPCParams, BatchSize>;
            }    // namespace commitments

            namespace algorithms {
                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static typename LPC::proof_type proof_eval(
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        const std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<std::vector<typename LPC::field_type::value_type>>>::type &evaluation_points,
                    //                    const std::array<std::vector<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> &evaluation_points,
                    typename LPC::precommitment_type &T,
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        const std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type &g,
                    // const std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size> &g,
                    const typename LPC::basic_fri::params_type &fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    using arr_vec_field_value_type = typename std::conditional<
                        LPC::leaf_size != 0,
                        std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        std::vector<std::vector<typename LPC::field_type::value_type>>>::type;
                    arr_vec_field_value_type z;

                    typename std::conditional<
                        LPC::leaf_size != 0,
                        std::array<std::vector<std::pair<typename LPC::field_type::value_type,
                                                         typename LPC::field_type::value_type>>,
                                   LPC::leaf_size>,
                        std::vector<std::vector<std::pair<typename LPC::field_type::value_type,
                                                          typename LPC::field_type::value_type>>>>::type
                        U_interpolation_points;

                    std::size_t leaf_size = g.size();
                    if constexpr (LPC::leaf_size == 0) {
                        z.resize(leaf_size);
                        U_interpolation_points.resize(leaf_size);
                    }
                    //                    std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>
                    //                    z; std::array<std::vector<std::pair<typename LPC::field_type::value_type,
                    //                                                     typename LPC::field_type::value_type>>,
                    //                               LPC::leaf_size>
                    //                        U_interpolation_points;

                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                        U_interpolation_points[polynom_index].resize(evaluation_points[polynom_index].size());
                        z[polynom_index].resize(evaluation_points[polynom_index].size());

                        for (std::size_t point_index = 0; point_index < evaluation_points[polynom_index].size();
                             point_index++) {

                            z[polynom_index][point_index] = g[polynom_index].evaluate(
                                evaluation_points[polynom_index][point_index]);    // transform to point-representation

                            U_interpolation_points[polynom_index][point_index] =
                                std::make_pair(evaluation_points[polynom_index][point_index],
                                               z[polynom_index][point_index]);    // prepare points for interpolation
                        }
                    }

                    typename std::conditional<
                        LPC::leaf_size != 0,
                        std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type Q;
                    if constexpr (LPC::leaf_size == 0) {
                        Q.resize(leaf_size);
                    }
                    //                    std::array<math::polynomial<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> Q;
                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {

                        math::polynomial<typename LPC::field_type::value_type> U =
                            math::lagrange_interpolation(U_interpolation_points[polynom_index]);

                        Q[polynom_index] = (g[polynom_index] - U);
                    }

                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                        math::polynomial<typename LPC::field_type::value_type> denominator_polynom = {1};
                        for (std::size_t point_index = 0; point_index < evaluation_points[polynom_index].size();
                             point_index++) {
                            denominator_polynom =
                                denominator_polynom * math::polynomial<typename LPC::field_type::value_type> {
                                                          -evaluation_points[polynom_index][point_index], 1};
                        }
                        Q[polynom_index] = Q[polynom_index] / denominator_polynom;
                    }

                    std::array<typename LPC::basic_fri::proof_type, LPC::lambda> fri_proof;

                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1; round_id++) {
                        fri_proof[round_id] = proof_eval<typename LPC::basic_fri>(Q, g, T, fri_params, transcript);
                    }

                    return typename LPC::proof_type({z, commit<typename LPC::basic_fri>(T), fri_proof});
                }

                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static typename LPC::proof_type proof_eval(
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        const std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<std::vector<typename LPC::field_type::value_type>>>::type &evaluation_points,
                    typename LPC::precommitment_type &T,
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        const std::array<math::polynomial_dfs<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<math::polynomial_dfs<typename LPC::field_type::value_type>>>::type &g,
                    //                    const std::array<math::polynomial_dfs<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> &g,
                    const typename LPC::basic_fri::params_type &fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    //                    std::array<math::polynomial<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> g_normal;
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type g_normal;
                    if constexpr (LPC::leaf_size == 0) {
                        g_normal.resize(g.size());
                    }

                    for (int polynom_index = 0; polynom_index < g.size(); ++polynom_index) {
                        g_normal[polynom_index] =
                            math::polynomial<typename LPC::field_type::value_type>(g[polynom_index].coefficients());
                    }

                    return proof_eval<LPC>(evaluation_points, T, g_normal, fri_params, transcript);
                }

                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static typename LPC::proof_type proof_eval(
                    const std::vector<typename LPC::field_type::value_type> &evaluation_points,
                    typename LPC::precommitment_type &T,
                    typename std::conditional<
                        LPC::leaf_size,
                        const std::array<math::polynomial_dfs<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<math::polynomial_dfs<typename LPC::field_type::value_type>>>::type &g,
                    //                    const std::array<math::polynomial_dfs<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> &g,
                    const typename LPC::basic_fri::params_type &fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size> z;
                    std::array<std::vector<std::pair<typename LPC::field_type::value_type,
                                                     typename LPC::field_type::value_type>>,
                               LPC::leaf_size>
                        U_interpolation_points;

                    std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size> g_normal;
                    for (int polynom_index = 0; polynom_index < g.size(); ++polynom_index) {
                        g_normal[polynom_index] =
                            math::polynomial<typename LPC::field_type::value_type>(g[polynom_index].coefficients());
                    }

                    for (std::size_t polynom_index = 0; polynom_index < LPC::leaf_size; polynom_index++) {
                        U_interpolation_points[polynom_index].resize(evaluation_points.size());
                        z[polynom_index].resize(evaluation_points.size());

                        for (std::size_t point_index = 0; point_index < evaluation_points.size(); point_index++) {

                            z[polynom_index][point_index] = g_normal[polynom_index].evaluate(
                                evaluation_points[point_index]);    // transform to point-representation

                            U_interpolation_points[polynom_index][point_index] =
                                std::make_pair(evaluation_points[point_index],
                                               z[polynom_index][point_index]);    // prepare points for interpolation
                        }
                    }

                    math::polynomial<typename LPC::field_type::value_type> denominator_polynom = {1};
                    for (std::size_t point_index = 0; point_index < evaluation_points.size(); point_index++) {
                        denominator_polynom =
                            denominator_polynom *
                            math::polynomial<typename LPC::field_type::value_type> {-evaluation_points[point_index], 1};
                    }

                    std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size> Q_normal;
                    for (std::size_t polynom_index = 0; polynom_index < LPC::leaf_size; polynom_index++) {

                        math::polynomial<typename LPC::field_type::value_type> U =
                            math::lagrange_interpolation(U_interpolation_points[polynom_index]);

                        Q_normal[polynom_index] = (g_normal[polynom_index] - U) / denominator_polynom;
                    }

                    std::array<typename LPC::basic_fri::proof_type, LPC::lambda> fri_proof;

                    std::array<math::polynomial_dfs<typename LPC::field_type::value_type>, LPC::leaf_size> Q;
                    for (int polynom_index = 0; polynom_index < Q_normal.size(); ++polynom_index) {
                        Q[polynom_index].from_coefficients(Q_normal[polynom_index]);
                        Q[polynom_index].resize(fri_params.D[0]->size());
                    }

                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1; round_id++) {
                        fri_proof[round_id] = typename LPC::basic_fri::proof_eval(Q, g, T, fri_params, transcript);
                    }

                    return proof_type({z, typename LPC::basic_fri::commit(T), fri_proof});
                }

                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static typename LPC::proof_type proof_eval(
                    const std::vector<typename LPC::field_type::value_type> &evaluation_points,
                    typename LPC::precommitment_type &T,
                    typename std::conditional<
                        LPC::leaf_size != 0,
                        const std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type &g,
                    //                    const std::array<math::polynomial<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> &g,
                    const typename LPC::basic_fri::params_type &fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    return proof_eval<LPC>(
                        std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>(
                            evaluation_points),
                        T,
                        g,
                        fri_params,
                        transcript);
                }

                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static bool verify_eval(
                    typename std::conditional<
                        (bool)LPC::leaf_size,
                        const std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        const std::vector<std::vector<typename LPC::field_type::value_type>>>::type
                        //                    const std::array<std::vector<typename LPC::field_type::value_type>,
                        //                    LPC::leaf_size>
                        &evaluation_points,
                    typename LPC::proof_type &proof,
                    typename LPC::basic_fri::params_type fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    std::size_t leaf_size = proof.z.size();
                    //                    std::array<std::vector<std::pair<typename LPC::field_type::value_type,
                    //                                                     typename LPC::field_type::value_type>>,
                    //                               LPC::leaf_size>
                    //                        U_interpolation_points;
                    typename std::conditional<
                        (bool)LPC::leaf_size,
                        std::array<std::vector<std::pair<typename LPC::field_type::value_type,
                                                         typename LPC::field_type::value_type>>,
                                   LPC::leaf_size>,
                        std::vector<std::vector<std::pair<typename LPC::field_type::value_type,
                                                          typename LPC::field_type::value_type>>>>::type
                        U_interpolation_points;

                    if constexpr (LPC::leaf_size == 0) {
                        U_interpolation_points.resize(leaf_size);
                    }

                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {

                        U_interpolation_points[polynom_index].resize(evaluation_points[polynom_index].size());

                        for (std::size_t point_index = 0; point_index < evaluation_points[polynom_index].size();
                             point_index++) {

                            U_interpolation_points[polynom_index][point_index] = std::make_pair(
                                evaluation_points[polynom_index][point_index], proof.z[polynom_index][point_index]);
                        }
                    }

                    //                    std::array<math::polynomial<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> U;
                    typename std::conditional<
                        (bool)LPC::leaf_size,
                        std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type U;
                    if constexpr (LPC::leaf_size == 0) {
                        U.resize(leaf_size);
                    }

                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                        U[polynom_index] = math::lagrange_interpolation(U_interpolation_points[polynom_index]);
                    }

                    //                    std::array<math::polynomial<typename LPC::field_type::value_type>,
                    //                    LPC::leaf_size> V;
                    typename std::conditional<
                        (bool)LPC::leaf_size,
                        std::array<math::polynomial<typename LPC::field_type::value_type>, LPC::leaf_size>,
                        std::vector<math::polynomial<typename LPC::field_type::value_type>>>::type V;
                    if constexpr (LPC::leaf_size == 0) {
                        V.resize(leaf_size);
                    }

                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                        V[polynom_index] = {1};
                        for (std::size_t point_index = 0; point_index < evaluation_points[polynom_index].size();
                             point_index++) {
                            V[polynom_index] =
                                V[polynom_index] * (math::polynomial<typename LPC::field_type::value_type>(
                                                       {-evaluation_points[polynom_index][point_index], 1}));
                        }
                    }

                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1; round_id++) {
                        if (!verify_eval<typename LPC::basic_fri>(
                                proof.fri_proof[round_id], fri_params, U, V, transcript)) {
                            return false;
                        }
                    }

                    return true;
                }

                template<typename LPC,
                         typename std::enable_if<
                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename LPC::field_type,
                                                                                             typename LPC::lpc_params,
                                                                                             LPC::leaf_size>,
                                             LPC>::value,
                             bool>::type = true>
                static bool verify_eval(
                    const std::vector<typename LPC::field_type::value_type> &evaluation_points,
                    typename LPC::proof_type &proof,
                    typename LPC::basic_fri::params_type fri_params,
                    typename LPC::basic_fri::transcript_type &transcript = typename LPC::basic_fri::transcript_type()) {

                    return verify_eval<LPC>(
                        std::array<std::vector<typename LPC::field_type::value_type>, LPC::leaf_size>(
                            evaluation_points),
                        proof,
                        fri_params,
                        transcript);
                }
            }    // namespace algorithms

            namespace algorithms {
                //                                template<typename LPC,
                //                                         typename std::enable_if<
                //                                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename
                //                                             LPC::field_type,
                //                                                                                                             typename
                //                                                                                                             LPC::lpc_params, 0>,
                //                                                             LPC>::value,
                //                                             bool>::type = true>
                //                                static typename LPC::proof_type proof_eval(
                //                                    const std::vector<std::vector<typename
                //                                    LPC::field_type::value_type>> &evaluation_points, typename
                //                                    LPC::precommitment_type &T, const
                //                                    std::vector<math::polynomial_dfs<typename
                //                                    LPC::field_type::value_type>> &g, const typename
                //                                    LPC::basic_fri::params_type &fri_params, typename
                //                                    LPC::basic_fri::transcript_type &transcript = typename
                //                                    LPC::basic_fri::transcript_type()) {
                //
                //                                    assert(evaluation_points.size() == g.size());
                //                                    std::size_t leaf_size = g.size();
                //
                //                                    std::vector<std::vector<typename LPC::field_type::value_type>>
                //                                    z(leaf_size);

                //                                    std::vector<std::vector<
                //                                        std::pair<typename LPC::field_type::value_type, typename
                //                                        LPC::field_type::value_type>>>
                //                                        U_interpolation_points(leaf_size);
                //
                //                                    std::vector<math::polynomial<typename
                //                                    LPC::field_type::value_type>> g_normal(leaf_size); for (int
                //                                    polynom_index = 0; polynom_index < leaf_size;
                //                                    ++polynom_index) {
                //                                        g_normal[polynom_index] =
                //                                            math::polynomial<typename
                //                                            LPC::field_type::value_type>(g[polynom_index].coefficients());
                //                                    }
                //
                //                                    for (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //                                        U_interpolation_points[polynom_index].resize(evaluation_points[polynom_index].size());
                //                                        z[polynom_index].resize(evaluation_points[polynom_index].size());
                //
                //                                        for (std::size_t point_index = 0; point_index <
                //                                        evaluation_points[polynom_index].size();
                //                                             point_index++) {
                //
                //                                            z[polynom_index][point_index] =
                //                                            g_normal[polynom_index].evaluate(
                //                                                evaluation_points[polynom_index][point_index]);    //
                //                                                transform to point-representation
                //
                //                                            U_interpolation_points[polynom_index][point_index] =
                //                                                std::make_pair(evaluation_points[polynom_index][point_index],
                //                                                               z[polynom_index][point_index]);    //
                //                                                               prepare points for interpolation
                //                                        }
                //                                    }
                //
                //                                    std::vector<math::polynomial<typename
                //                                    LPC::field_type::value_type>> Q_normal(leaf_size); for
                //                                    (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //
                //                                        math::polynomial<typename LPC::field_type::value_type> U =
                //                                            math::lagrange_interpolation(U_interpolation_points[polynom_index]);
                //
                //                                        Q_normal[polynom_index] = (g_normal[polynom_index] - U);
                //                                    }
                //
                //                                    for (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //                                        math::polynomial<typename LPC::field_type::value_type>
                //                                        denominator_polynom = {1}; for (std::size_t point_index = 0;
                //                                        point_index < evaluation_points[polynom_index].size();
                //                                             point_index++) {
                //                                            denominator_polynom =
                //                                                denominator_polynom * math::polynomial<typename
                //                                                LPC::field_type::value_type> {
                //                                                                          -evaluation_points[polynom_index][point_index],
                //                                                                          1};
                //                                        }
                //                                        Q_normal[polynom_index] = Q_normal[polynom_index] /
                //                                        denominator_polynom;
                //                                    }
                //
                //                                    std::array<typename LPC::basic_fri::proof_type, LPC::lambda>
                //                                    fri_proof;
                //
                //                                    std::vector<math::polynomial_dfs<typename
                //                                    LPC::field_type::value_type>> Q(leaf_size); for (int polynom_index
                //                                    = 0; polynom_index < Q_normal.size();
                //                                    ++polynom_index) {
                //                                        Q[polynom_index].from_coefficients(Q_normal[polynom_index]);
                //                                    }
                //
                //                                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1;
                //                                    round_id++) {
                //                                        fri_proof[round_id] = proof_eval<typename LPC::basic_fri>(Q,
                //                                        g, T, fri_params, transcript);
                //                                    }
                //
                //                                    return typename LPC::proof_type({z, commit<typename
                //                                    LPC::basic_fri>(T), fri_proof});
                //                                }
                //
                //                template<typename LPC, typename std::enable_if<
                //                                           std::is_base_of<commitments::batched_list_polynomial_commitment<
                //                                                               typename LPC::field_type, typename
                //                                                               LPC::lpc_params, 0>,
                //                                                           LPC>::value,
                //                                           bool>::type = true>
                //                static typename LPC::proof_type proof_eval(
                //                    const std::vector<std::vector<typename LPC::field_type::value_type>>
                //                    &evaluation_points, typename LPC::precommitment_type &T, const
                //                    std::vector<math::polynomial<typename LPC::field_type::value_type>> &g, const
                //                    typename LPC::basic_fri::params_type &fri_params, typename
                //                    LPC::basic_fri::transcript_type &transcript = typename
                //                    LPC::basic_fri::transcript_type()) {
                //
                //                    assert(evaluation_points.size() == g.size());
                //                    std::size_t leaf_size = g.size();
                //
                //                    std::vector<std::vector<typename LPC::field_type::value_type>> z(leaf_size);
                //                    std::vector<std::vector<
                //                        std::pair<typename LPC::field_type::value_type, typename
                //                        LPC::field_type::value_type>>> U_interpolation_points(leaf_size);
                //
                //                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                //                        U_interpolation_points[polynom_index].resize(evaluation_points[polynom_index].size());
                //                        z[polynom_index].resize(evaluation_points[polynom_index].size());
                //
                //                        for (std::size_t point_index = 0; point_index <
                //                        evaluation_points[polynom_index].size();
                //                             point_index++) {
                //
                //                            z[polynom_index][point_index] = g[polynom_index].evaluate(
                //                                evaluation_points[polynom_index][point_index]);    // transform to
                //                                point - representation
                //
                //                                        U_interpolation_points[polynom_index][point_index] =
                //                                std::make_pair(evaluation_points[polynom_index][point_index],
                //                                               z[polynom_index][point_index]);    // prepare points
                //                                               for interpolation
                //                        }
                //                    }
                //
                //                    std::vector<math::polynomial<typename LPC::field_type::value_type>> Q(leaf_size);
                //                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                //
                //                        math::polynomial<typename LPC::field_type::value_type> U =
                //                            math::lagrange_interpolation(U_interpolation_points[polynom_index]);
                //
                //                        Q[polynom_index] = (g[polynom_index] - U);
                //                    }
                //
                //                    for (std::size_t polynom_index = 0; polynom_index < leaf_size; polynom_index++) {
                //                        math::polynomial<typename LPC::field_type::value_type> denominator_polynom =
                //                        {1}; for (std::size_t point_index = 0; point_index <
                //                        evaluation_points[polynom_index].size();
                //                             point_index++) {
                //                            denominator_polynom =
                //                                denominator_polynom * math::polynomial<typename
                //                                LPC::field_type::value_type> {
                //                                                          -evaluation_points[polynom_index][point_index],
                //                                                          1};
                //                        }
                //                        Q[polynom_index] = Q[polynom_index] / denominator_polynom;
                //                    }
                //
                //                    std::array<typename LPC::basic_fri::proof_type, LPC::lambda> fri_proof;
                //
                //                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1; round_id++) {
                //                        fri_proof[round_id] = proof_eval<typename LPC::basic_fri>(Q, g, T, fri_params,
                //                        transcript);
                //                    }
                //
                //                    return typename LPC::proof_type({z, commit<typename LPC::basic_fri>(T),
                //                    fri_proof});
                //                }
                //
                //                                template<typename LPC,
                //                                         typename std::enable_if<
                //                                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename
                //                                             LPC::field_type,
                //                                                                                                             typename
                //                                                                                                             LPC::lpc_params, 0>,
                //                                                             LPC>::value,
                //                                             bool>::type = true>
                //                                static typename LPC::proof_type proof_eval(
                //                                    const std::vector<typename LPC::field_type::value_type>
                //                                    &evaluation_points, typename LPC::precommitment_type &T, const
                //                                    std::vector<math::polynomial_dfs<typename
                //                                    LPC::field_type::value_type>> &g, const typename
                //                                    LPC::basic_fri::params_type &fri_params, typename
                //                                    LPC::basic_fri::transcript_type &transcript = typename
                //                                    LPC::basic_fri::transcript_type()) {
                //
                //                                    std::size_t leaf_size = g.size();
                //                                    return proof_eval<LPC>(
                //                                        std::vector<std::vector<typename
                //                                        LPC::field_type::value_type>>(leaf_size, evaluation_points),
                //                                        T, g, fri_params, transcript);
                //                                }
                //
                //                                template<typename LPC,
                //                                         typename std::enable_if<
                //                                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename
                //                                             LPC::field_type,
                //                                                                                                             typename
                //                                                                                                             LPC::lpc_params, 0>,
                //                                                             LPC>::value,
                //                                             bool>::type = true>
                //                                static typename LPC::proof_type proof_eval(
                //                                    const std::vector<typename LPC::field_type::value_type>
                //                                    &evaluation_points, typename LPC::precommitment_type &T, const
                //                                    std::vector<math::polynomial<typename
                //                                    LPC::field_type::value_type>> &g, const typename
                //                                    LPC::basic_fri::params_type &fri_params, typename
                //                                    LPC::basic_fri::transcript_type &transcript = typename
                //                                    LPC::basic_fri::transcript_type()) {
                //
                //                                    std::size_t leaf_size = g.size();
                //                                    return proof_eval<LPC>(
                //                                        std::vector<std::vector<typename
                //                                        LPC::field_type::value_type>>(leaf_size, evaluation_points),
                //                                        T, g, fri_params, transcript);
                //                                }
                //
                //                                template<typename LPC,
                //                                         typename std::enable_if<
                //                                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename
                //                                             LPC::field_type,
                //                                                                                                             typename
                //                                                                                                             LPC::lpc_params, 0>,
                //                                                             LPC>::value,
                //                                             bool>::type = true>
                //                                static bool verify_eval(
                //                                    const std::vector<std::vector<typename
                //                                    LPC::field_type::value_type>> &evaluation_points, typename
                //                                    LPC::proof_type &proof, typename LPC::basic_fri::params_type
                //                                    fri_params, typename LPC::basic_fri::transcript_type &transcript =
                //                                    typename LPC::basic_fri::transcript_type()) {
                //
                //                                    std::size_t leaf_size = proof.z.size();
                //
                //                                    std::vector<std::vector<
                //                                        std::pair<typename LPC::field_type::value_type, typename
                //                                        LPC::field_type::value_type>>>
                //                                        U_interpolation_points(leaf_size);
                //
                //                                    for (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //
                //                                        U_interpolation_points[polynom_index].resize(evaluation_points[polynom_index].size());
                //
                //                                        for (std::size_t point_index = 0; point_index <
                //                                        evaluation_points[polynom_index].size();
                //                                             point_index++) {
                //
                //                                            U_interpolation_points[polynom_index][point_index] =
                //                                            std::make_pair(
                //                                                evaluation_points[polynom_index][point_index],
                //                                                proof.z[polynom_index][point_index]);
                //                                        }
                //                                    }
                //
                //                                    std::vector<math::polynomial<typename
                //                                    LPC::field_type::value_type>> U(leaf_size);
                //
                //                                    for (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //                                        U[polynom_index] =
                //                                        math::lagrange_interpolation(U_interpolation_points[polynom_index]);
                //                                    }
                //
                //                                    std::vector<math::polynomial<typename
                //                                    LPC::field_type::value_type>> V(leaf_size);
                //
                //                                    for (std::size_t polynom_index = 0; polynom_index < leaf_size;
                //                                    polynom_index++) {
                //                                        V[polynom_index] = {1};
                //                                        for (std::size_t point_index = 0; point_index <
                //                                        evaluation_points[polynom_index].size();
                //                                             point_index++) {
                //                                            V[polynom_index] =
                //                                                V[polynom_index] * (math::polynomial<typename
                //                                                LPC::field_type::value_type>(
                //                                                                       {-evaluation_points[polynom_index][point_index],
                //                                                                       1}));
                //                                        }
                //                                    }
                //
                //                                    for (std::size_t round_id = 0; round_id <= LPC::lambda - 1;
                //                                    round_id++) {
                //                                        if (!verify_eval<typename LPC::basic_fri>(
                //                                                proof.fri_proof[round_id], fri_params, U, V,
                //                                                transcript)) {
                //                                            return false;
                //                                        }
                //                                    }
                //
                //                                    return true;
                //                                }
                //
                //                                template<typename LPC,
                //                                         typename std::enable_if<
                //                                             std::is_base_of<commitments::batched_list_polynomial_commitment<typename
                //                                             LPC::field_type,
                //                                                                                                             typename
                //                                                                                                             LPC::lpc_params, 0>,
                //                                                             LPC>::value,
                //                                             bool>::type = true>
                //                                static bool verify_eval(
                //                                    const std::vector<typename LPC::field_type::value_type>
                //                                    &evaluation_points, typename LPC::proof_type &proof, typename
                //                                    LPC::basic_fri::params_type fri_params, typename
                //                                    LPC::basic_fri::transcript_type &transcript = typename
                //                                    LPC::basic_fri::transcript_type()) {
                //
                //                                    std::size_t leaf_size = proof.z.size();
                //                                    return verify_eval<LPC>(
                //                                        std::vector<std::vector<typename
                //                                        LPC::field_type::value_type>>(leaf_size, evaluation_points),
                //                                        proof, fri_params, transcript);
                //                                }
            }    // namespace algorithms
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_BATCHED_LIST_POLYNOMIAL_COMMITMENT_SCHEME_HPP
