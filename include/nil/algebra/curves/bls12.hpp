//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef ALGEBRA_CURVES_BLS12_HPP
#define ALGEBRA_CURVES_BLS12_HPP

#include <nil/crypto3/algebra/curves/curve_weierstrass.hpp>
#include <nil/crypto3/algebra/curves/detail/element/bls12.hpp>

namespace nil {
    namespace algebra {
    	namespace curves {
	    	/*
				E/Fp: y^2 = x^3 + 4.
			*/

            template<std::size_t ModulusBits>
            struct bls12 {

            };

            template<>
	        struct bls12<381> : public curve_weierstrass<fields::bls12_381_fq<381>> {
	        private:
	        	typedef typename curve_weierstrass<fields::bls12_381_fq<381>> policy_type;
	        public:
		        typedef typename policy_type::number_type number_type;
		        typedef typename policy_type::field_type field_type;

		        constexpr static const number_type p = 
		        	0x1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab_cppui381;
		        constexpr static const number_type a = 0;
		        constexpr static const number_type b = 0x04;
		        constexpr static const number_type x = 
		        	0x17f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb_cppui381;
		        constexpr static const number_type y = 
		        	0x08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1_cppui381;

                constexpr static const std::size_t subgroup_order_size = 255;
		        constexpr static const number_type subgroup_order =
		        	0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001_cppui255;

		        typedef typename detail::element_bls12<field_type::value_type, 381> value_type;
	    	};

            typedef bls12<381> bls12_381;
    	}    // namespace curves
    }    // namespace algebra
}    // namespace nil

#endif    // ALGEBRA_CURVES_BLS12_381_HPP
