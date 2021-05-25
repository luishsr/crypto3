//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2020 Ilias Khairullin <ilias@nil.foundation>
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

#ifndef CRYPTO3_ALGEBRA_CURVES_ALT_BN128_G2_ELEMENT_HPP
#define CRYPTO3_ALGEBRA_CURVES_ALT_BN128_G2_ELEMENT_HPP

#include <nil/crypto3/algebra/curves/detail/alt_bn128/basic_policy.hpp>
#include <nil/crypto3/algebra/curves/detail/alt_bn128/g1.hpp>
#include <nil/crypto3/algebra/curves/detail/scalar_mul.hpp>

#include <nil/crypto3/detail/type_traits.hpp>
#include <nil/crypto3/detail/literals.hpp>

namespace nil {
    namespace crypto3 {
        namespace algebra {
            namespace curves {
                namespace detail {

                    /** @brief A struct representing a group G2 of Barreto-Naehrig curve.
                     *    @tparam Version size of the base field in bits
                     *
                     */
                    template<std::size_t Version>
                    struct alt_bn128_g2;

                    /** @brief A struct representing an element from the group G2 of Barreto-Naehrig curve.
                     *    @tparam Version size of the base field in bits
                     *
                     */
                    template<std::size_t Version>
                    struct element_alt_bn128_g2 { };

                    /** @brief A struct representing an elememnt from the group G2 of Barreto-Naehrig curve.
                     *
                     * The size of the group G2 in bits equals 254.
                     */
                    template<>
                    struct element_alt_bn128_g2<254> {

                        using group_type = alt_bn128_g2<254>;

                        using policy_type = alt_bn128_basic_policy<254>;

                        typedef typename policy_type::g1_field_type::value_type g1_field_type_value;
                        typedef typename policy_type::g2_field_type::value_type g2_field_type_value;

                        using underlying_field_value_type = g2_field_type_value;

                        underlying_field_value_type X;
                        underlying_field_value_type Y;
                        underlying_field_value_type Z;

                        /*************************  Constructors and zero/one  ***********************************/

                        /** @brief
                         *    @return the point at infinity by default
                         *
                         */
                        element_alt_bn128_g2() :
                            element_alt_bn128_g2(underlying_field_value_type::zero(),
                                                 underlying_field_value_type::one(),
                                                 underlying_field_value_type::zero()) {};
                        // must be
                        // element_alt_bn128_g2() : element_alt_bn128_g2(zero_fill[0], zero_fill[1], zero_fill[2]) {};
                        // when constexpr fields will be finished

                        /** @brief
                         *    @return the selected point $(X:Y:Z)$
                         *
                         */
                        element_alt_bn128_g2(underlying_field_value_type X, underlying_field_value_type Y,
                                             underlying_field_value_type Z) {
                            this->X = X;
                            this->Y = Y;
                            this->Z = Z;
                        };

                        /** @brief Get the point at infinity
                         *
                         */
                        static element_alt_bn128_g2 zero() {
                            return element_alt_bn128_g2();
                        }

                        /** @brief Get the generator of group G2
                         *
                         */
                        static element_alt_bn128_g2 one() {
                            return element_alt_bn128_g2(one_fill[0], one_fill[1], one_fill[2]);
                        }

                        bool operator==(const element_alt_bn128_g2 &other) const {
                            if (this->is_zero()) {
                                return other.is_zero();
                            }

                            if (other.is_zero()) {
                                return false;
                            }

                            /* now neither is O */

                            // using Jacobian coordinates so:
                            // (X1:Y1:Z1) = (X2:Y2:Z2)
                            // iff
                            // X1/Z1^2 == X2/Z2^2 and Y1/Z1^3 == Y2/Z2^3
                            // iff
                            // X1 * Z2^2 == X2 * Z1^2 and Y1 * Z2^3 == Y2 * Z1^3

                            underlying_field_value_type Z1_squared = (this->Z).squared();
                            underlying_field_value_type Z2_squared = (other.Z).squared();

                            if ((this->X * Z2_squared) != (other.X * Z1_squared)) {
                                return false;
                            }

                            underlying_field_value_type Z1_cubed = (this->Z) * Z1_squared;
                            underlying_field_value_type Z2_cubed = (other.Z) * Z2_squared;

                            if ((this->Y * Z2_cubed) != (other.Y * Z1_cubed)) {
                                return false;
                            }

                            return true;
                        }

                        bool operator!=(const element_alt_bn128_g2 &other) const {
                            return !(operator==(other));
                        }

                        /** @brief
                         *
                         * @return true if element from group G2 is the point at infinity
                         */
                        bool is_zero() const {
                            return (this->Z.is_zero());
                        }

                        /** @brief
                         *
                         * @return true if element from group G2 in affine coordinates
                         */
                        bool is_special() const {
                            return (this->is_zero() || this->Z == underlying_field_value_type::one());
                        }

                        /*************************  Arithmetic operations  ***********************************/

                        element_alt_bn128_g2 operator=(const element_alt_bn128_g2 &other) {
                            // handle special cases having to do with O
                            this->X = other.X;
                            this->Y = other.Y;
                            this->Z = other.Z;

                            return *this;
                        }

                        element_alt_bn128_g2 operator+(const element_alt_bn128_g2 &other) const {
                            // handle special cases having to do with O
                            if (this->is_zero()) {
                                return other;
                            }

                            if (other.is_zero()) {
                                return (*this);
                            }

                            if (*this == other) {
                                return this->doubled();
                            }

                            return this->add(other);
                        }

                        element_alt_bn128_g2 operator-() const {
                            return element_alt_bn128_g2(this->X, -(this->Y), this->Z);
                        }

                        element_alt_bn128_g2 operator-(const element_alt_bn128_g2 &other) const {
                            return (*this) + (-other);
                        }

                        /** @brief
                         *
                         * @return doubled element from group G2
                         */
                        element_alt_bn128_g2 doubled() const {
                            // handle point at infinity
                            if (this->is_zero()) {
                                return (*this);
                            }

                            // NOTE: does not handle O and pts of order 2,4
                            // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-projective.html#doubling-dbl-2007-bl

                            underlying_field_value_type A = (this->X).squared();    // A = X1^2
                            underlying_field_value_type B = (this->Y).squared();    // B = Y1^2
                            underlying_field_value_type C = B.squared();            // C = B^2
                            underlying_field_value_type D = (this->X + B).squared() - A - C;
                            D = D + D;                                           // D = 2 * ((X1 + B)^2 - A - C)
                            underlying_field_value_type E = A.doubled() + A;     // E = 3 * A
                            underlying_field_value_type F = E.squared();         // F = E^2
                            underlying_field_value_type X3 = F - D.doubled();    // X3 = F - 2 D
                            underlying_field_value_type eightC = C.doubled().doubled().doubled();
                            underlying_field_value_type Y3 = E * (D - X3) - eightC;    // Y3 = E * (D - X3) - 8 * C
                            underlying_field_value_type Y1Z1 = (this->Y) * (this->Z);
                            underlying_field_value_type Z3 = Y1Z1 + Y1Z1;    // Z3 = 2 * Y1 * Z1

                            return element_alt_bn128_g2(X3, Y3, Z3);
                        }

                        /** @brief
                         *
                         * “Mixed addition” refers to the case Z2 known to be 1.
                         * @return addition of two elements from group G2
                         */
                        element_alt_bn128_g2 mixed_add(const element_alt_bn128_g2 &other) const {

                            // handle special cases having to do with O
                            if (this->is_zero()) {
                                return other;
                            }

                            if (other.is_zero()) {
                                return *this;
                            }

                            // no need to handle points of order 2,4
                            // (they cannot exist in a prime-order subgroup)

                            // check for doubling case

                            // using Jacobian coordinates so:
                            // (X1:Y1:Z1) = (X2:Y2:Z2)
                            // iff
                            // X1/Z1^2 == X2/Z2^2 and Y1/Z1^3 == Y2/Z2^3
                            // iff
                            // X1 * Z2^2 == X2 * Z1^2 and Y1 * Z2^3 == Y2 * Z1^3

                            // we know that Z2 = 1

                            const underlying_field_value_type Z1Z1 = (this->Z).squared();

                            const underlying_field_value_type &U1 = this->X;
                            const underlying_field_value_type U2 = other.X * Z1Z1;

                            const underlying_field_value_type Z1_cubed = (this->Z) * Z1Z1;

                            const underlying_field_value_type &S1 = (this->Y);              // S1 = Y1 * Z2 * Z2Z2
                            const underlying_field_value_type S2 = (other.Y) * Z1_cubed;    // S2 = Y2 * Z1 * Z1Z1

                            if (U1 == U2 && S1 == S2) {
                                // dbl case; nothing of above can be reused
                                return this->doubled();
                            }

                            // NOTE: does not handle O and pts of order 2,4
                            // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-madd-2007-bl
                            underlying_field_value_type H = U2 - (this->X);    // H = U2-X1
                            underlying_field_value_type HH = H.squared();      // HH = H&2
                            underlying_field_value_type I = HH + HH;           // I = 4*HH
                            I = I + I;
                            underlying_field_value_type J = H * I;             // J = H*I
                            underlying_field_value_type r = S2 - (this->Y);    // r = 2*(S2-Y1)
                            r = r + r;
                            underlying_field_value_type V = (this->X) * I;               // V = X1*I
                            underlying_field_value_type X3 = r.squared() - J - V - V;    // X3 = r^2-J-2*V
                            underlying_field_value_type Y3 = (this->Y) * J;              // Y3 = r*(V-X3)-2*Y1*J
                            Y3 = r * (V - X3) - Y3 - Y3;
                            underlying_field_value_type Z3 =
                                ((this->Z) + H).squared() - Z1Z1 - HH;    // Z3 = (Z1+H)^2-Z1Z1-HH

                            return element_alt_bn128_g2(X3, Y3, Z3);
                        }

                    private:
                        element_alt_bn128_g2 add(const element_alt_bn128_g2 &other) const {

                            // NOTE: does not handle O and pts of order 2,4
                            // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-projective.html#addition-add-1998-cmo-2

                            underlying_field_value_type Z1Z1 = (this->Z).squared();           // Z1Z1 = Z1^2
                            underlying_field_value_type Z2Z2 = (other.Z).squared();           // Z2Z2 = Z2^2
                            underlying_field_value_type U1 = (this->X) * Z2Z2;                // U1 = X1 * Z2Z2
                            underlying_field_value_type U2 = (other.X) * Z1Z1;                // U2 = X2 * Z1Z1
                            underlying_field_value_type S1 = (this->Y) * (other.Z) * Z2Z2;    // S1 = Y1 * Z2 * Z2Z2
                            underlying_field_value_type S2 = (other.Y) * (this->Z) * Z1Z1;    // S2 = Y2 * Z1 * Z1Z1
                            underlying_field_value_type H = U2 - U1;                          // H = U2-U1
                            underlying_field_value_type S2_minus_S1 = S2 - S1;
                            underlying_field_value_type I = H.doubled().squared();             // I = (2 * H)^2
                            underlying_field_value_type J = H * I;                             // J = H * I
                            underlying_field_value_type r = S2_minus_S1.doubled();             // r = 2 * (S2-S1)
                            underlying_field_value_type V = U1 * I;                            // V = U1 * I
                            underlying_field_value_type X3 = r.squared() - J - V.doubled();    // X3 = r^2 - J - 2 * V
                            underlying_field_value_type S1_J = S1 * J;
                            underlying_field_value_type Y3 = r * (V - X3) - S1_J.doubled();    // Y3 = r * (V-X3)-2 S1 J
                            underlying_field_value_type Z3 =
                                ((this->Z + other.Z).squared() - Z1Z1 - Z2Z2) * H;    // Z3 = ((Z1+Z2)^2-Z1Z1-Z2Z2) * H

                            return element_alt_bn128_g2(X3, Y3, Z3);
                        }

                    public:
                        /*************************  Extra arithmetic operations  ***********************************/
                    private:    // probably shoould be public for pairing
                        element_alt_bn128_g2 mul_by_q() const {
                            return element_alt_bn128_g2(twist_mul_by_q_X * (this->X).Frobenius_map(1),
                                                        twist_mul_by_q_Y * (this->Y).Frobenius_map(1),
                                                        (this->Z).Frobenius_map(1));
                        }

                    public:
                        /*************************  Reducing operations  ***********************************/

                        /** @brief
                         *
                         * @return return the corresponding element from group G2 in affine coordinates
                         */
                        element_alt_bn128_g2 to_affine() {
                            underlying_field_value_type p_out[3];

                            if (this->is_zero()) {
                                p_out[0] = underlying_field_value_type::zero();
                                p_out[1] = underlying_field_value_type::one();
                                p_out[2] = underlying_field_value_type::zero();
                            } else {
                                underlying_field_value_type Z_inv = this->Z.inversed();
                                underlying_field_value_type Z2_inv = Z_inv.squared();
                                underlying_field_value_type Z3_inv = Z2_inv * Z_inv;
                                p_out[0] = this->X * Z2_inv;
                                p_out[1] = this->Y * Z3_inv;
                                p_out[2] = underlying_field_value_type::one();
                            }

                            return element_alt_bn128_g2(p_out[0], p_out[1], p_out[2]);
                        }

                        /** @brief
                         *
                         * @return return the corresponding element from group G2 in affine coordinates
                         */
                        element_alt_bn128_g2 to_projective() {
                            return this->to_affine();
                        }

                    private:
                        constexpr static const g1_field_type_value a = g1_field_type_value(policy_type::a);
                        constexpr static const g1_field_type_value b = g1_field_type_value(policy_type::b);

                        constexpr static const g2_field_type_value twist = g2_field_type_value(0x09, 0x01);

                        constexpr static const g2_field_type_value twist_coeff_b = b * twist.inversed();

                        constexpr static const g1_field_type_value twist_mul_by_b_c0 = b * g2_field_type_value::non_residue;
                        constexpr static const g1_field_type_value twist_mul_by_b_c1 = b * g2_field_type_value::non_residue;
                        constexpr static const g2_field_type_value twist_mul_by_q_X = g2_field_type_value(
                            0x2FB347984F7911F74C0BEC3CF559B143B78CC310C2C3330C99E39557176F553D_cppui254,
                            0x16C9E55061EBAE204BA4CC8BD75A079432AE2A1D0B7C9DCE1665D51C640FCBA2_cppui253);
                        constexpr static const g2_field_type_value twist_mul_by_q_Y = g2_field_type_value(
                            0x63CF305489AF5DCDC5EC698B6E2F9B9DBAAE0EDA9C95998DC54014671A0135A_cppui251,
                            0x7C03CBCAC41049A0704B5A7EC796F2B21807DC98FA25BD282D37F632623B0E3_cppui251);

                        constexpr static const std::array<underlying_field_value_type, 3> zero_fill = {
                            underlying_field_value_type::zero(), underlying_field_value_type::one(),
                            underlying_field_value_type::zero()};

                        constexpr static const std::array<underlying_field_value_type, 3> one_fill = {
                            underlying_field_value_type(
                                0x1800DEEF121F1E76426A00665E5C4479674322D4F75EDADD46DEBD5CD992F6ED_cppui254,
                                0x198E9393920D483A7260BFB731FB5D25F1AA493335A9E71297E485B7AEF312C2_cppui254),
                            underlying_field_value_type(
                                0x12C85EA5DB8C6DEB4AAB71808DCB408FE3D1E7690C43D37B4CE6CC0166FA7DAA_cppui254,
                                0x90689D0585FF075EC9E99AD690C3395BC4B313370B38EF355ACDADCD122975B_cppui254),
                            underlying_field_value_type::one()};
                    };

                    constexpr std::array<typename element_alt_bn128_g2<254>::underlying_field_value_type, 3> const
                        element_alt_bn128_g2<254>::zero_fill;

                    constexpr std::array<typename element_alt_bn128_g2<254>::underlying_field_value_type, 3> const
                        element_alt_bn128_g2<254>::one_fill;
                }    // namespace detail
            }        // namespace curves
        }            // namespace algebra
    }                // namespace crypto3
}    // namespace nil
#endif    // CRYPTO3_ALGEBRA_CURVES_ALT_BN128_G2_ELEMENT_HPP
