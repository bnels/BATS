#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <vector>
#include <set>
#include <random>

#include <bats.hpp>

#define F2 ModP<int, 2>
#define F3 ModP<int, 3>
#define Q Rational<int>
#define F5 ModP<int, 5>

#define N_SEEDS 4

TEST_CASE_TEMPLATE("Col Matrix Solve", F, F2, F3, F5, Q, int) {

	using VT = SparseVector<F, size_t>;
	using MatT = ColumnMatrix<VT>;

	MatT I = MatT::identity(5);

	std::vector<size_t> ind = {0,2,3};
    std::vector<F> val = {-1, 1, -1};

	VT y(ind, val);

	SUBCASE("U solve") {
		auto x1 = u_solve(I, y);

		REQUIRE(x1 == y);
	}

	SUBCASE("L solve") {
		auto x2 = l_solve(I, y);

		REQUIRE(x2 == y);
	}

}

TEST_CASE_TEMPLATE("Shapes", F, F2, F3, F5, Q) {
	using VT = SparseVector<F, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Identity") {
		MatT I = MatT::identity(5);

		CHECK(I.is_upper());
		CHECK(I.is_lower());
		CHECK(I.is_pivot_matrix());
		CHECK(I.is_EL());
		CHECK(I.is_ELhat());
		CHECK(I.is_EU());
		CHECK(I.is_EUhat());
	}

	SUBCASE("Upper") {
		std::vector<VT> col{VT({0}, {1}), VT({0,1}, {1,1})};
		MatT U(2,2,col);
		CHECK(U.is_upper());
		CHECK(!(U.is_lower()));
	}
	SUBCASE("Lower") {
		std::vector<VT> col{VT({0,1}, {1,1}), VT({1}, {1})};
		MatT L(2,2,col);
		CHECK(L.is_lower());
		CHECK(!(L.is_upper()));
	}
}

#define CHECK_LEUP(F, A) \
CHECK(F.LEUP_prod() == A); \
CHECK(F.L.is_lower()); \
CHECK(F.U.is_upper()); \
CHECK(F.E.is_EL()); \
CHECK(F.P.is_pivot_matrix());

TEST_CASE_TEMPLATE("LEUP Factorization", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 10, 0.2, 1, generator);

			auto F = LEUP(A);
			CHECK_LEUP(F, A)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 20, 0.2, 1, generator);

			auto F = LEUP(A);
			CHECK_LEUP(F, A)
		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(20, 10, 0.2, 1, generator);

			auto F = LEUP(A);
			CHECK_LEUP(F, A)
		}
	}

}

#define CHECK_PUEL(F, A) \
CHECK(F.PUEL_prod() == A); \
CHECK(F.L.is_lower()); \
CHECK(F.U.is_upper()); \
CHECK(F.E.is_ELhat()); \
CHECK(F.P.is_pivot_matrix());

TEST_CASE_TEMPLATE("PUEL Factorization", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 10, 0.2, 1, generator);

			auto F = PUEL(A);
			CHECK_PUEL(F, A)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 20, 0.2, 1, generator);

			auto F = PUEL(A);
			CHECK_PUEL(F, A)
		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(20, 10, 0.2, 1, generator);

			auto F = PUEL(A);
			CHECK_PUEL(F, A)
		}
	}

}


#define CHECK_PLEU(F, A) \
CHECK(F.PLEU_prod() == A); \
CHECK(F.L.is_lower()); \
CHECK(F.U.is_upper()); \
CHECK(F.E.is_EU()); \
CHECK(F.P.is_pivot_matrix());

TEST_CASE_TEMPLATE("PLEU Factorization", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 10, 0.2, 1, generator);

			auto F = PLEU(A);
			CHECK_PLEU(F, A)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 20, 0.2, 1, generator);

			auto F = PLEU(A);
			CHECK_PLEU(F, A)
		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(20, 10, 0.2, 1, generator);

			auto F = PLEU(A);
			CHECK_PLEU(F, A)
		}
	}

}

#define CHECK_UELP(F, A) \
CHECK(F.UELP_prod() == A); \
CHECK(F.L.is_lower()); \
CHECK(F.U.is_upper()); \
CHECK(F.E.is_EUhat()); \
CHECK(F.P.is_pivot_matrix());

TEST_CASE_TEMPLATE("UELP Factorization", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 10, 0.2, 1, generator);

			auto F = UELP(A);
			CHECK_UELP(F, A)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 20, 0.2, 1, generator);

			auto F = UELP(A);
			CHECK_UELP(F, A)
		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(20, 10, 0.2, 1, generator);

			auto F = UELP(A);
			CHECK_UELP(F, A)
		}
	}

}

#define CHECK_LQU(F, A) \
CHECK(F.LQU_prod() == A); \
CHECK(F.L.is_lower()); \
CHECK(F.U.is_upper()); \
CHECK(F.E.is_pivot_matrix()); \


TEST_CASE_TEMPLATE("LQU Factorization", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 10, 0.2, 1, generator);

			auto F = LQU(A);
			CHECK_LQU(F, A)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(10, 20, 0.2, 1, generator);

			auto F = LQU(A);
			CHECK_LQU(F, A)
		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			auto A = MatT::random(20, 10, 0.2, 1, generator);

			auto F = LQU(A);
			CHECK_LQU(F, A)
		}
	}

}

#define CHECK_EL_L(m, n, p, p2, generator) \
SUBCASE("EL - L") {\
	auto A = MatT::random(m, n, p, 1, generator); \
	auto B = MatT::random(n, n, p2, 1, generator); \
	auto F = LEUP(A); \
	CHECK_LEUP(F, A) \
	auto EL = F.E; \
	auto FB = LQU(B); \
	CHECK_LQU(FB, B) \
	auto L = FB.L;  \
	auto Ltil = EL_L_commute(EL, L); \
	CHECK(Ltil.is_lower()); \
	CHECK(Ltil * EL == EL * L); \
}

#define CHECK_L_EL(m, n, p, p2, generator) \
SUBCASE("L - ELhat") {\
	auto A = MatT::random(m, n, p, 1, generator); \
	auto B = MatT::random(m, m, p2, 1, generator); \
	auto F = PUEL(A); \
	CHECK_PUEL(F, A) \
	auto EL = F.E; \
	auto FB = LQU(B); \
	CHECK_LQU(FB, B) \
	auto L = FB.L;  \
	auto Ltil = L_EL_commute(L, EL); \
	CHECK(Ltil.is_lower()); \
	CHECK(EL * Ltil == L * EL); \
}

#define CHECK_U_EU(m, n, p, p2, generator) \
SUBCASE("U - EU") {\
	auto A = MatT::random(m, n, p, 1, generator); \
	auto B = MatT::random(m, m, p2, 1, generator); \
	auto F = PLEU(A); \
	CHECK_PLEU(F, A) \
	auto EU = F.E; \
	auto FB = LQU(B); \
	CHECK_LQU(FB, B) \
	auto U = FB.U;  \
	auto Util = U_EU_commute(U, EU); \
	CHECK(Util.is_upper()); \
	CHECK(U * EU == EU * Util); \
}

#define CHECK_EU_U(m, n, p, p2, generator) \
SUBCASE("EUhat - U") {\
	auto A = MatT::random(m, n, p, 1, generator); \
	auto B = MatT::random(n, n, p2, 1, generator); \
	auto F = UELP(A); \
	CHECK_UELP(F, A) \
	auto EU = F.E; \
	auto FB = LQU(B); \
	CHECK_LQU(FB, B) \
	auto U = FB.U;  \
	auto Util = EU_U_commute(EU, U); \
	CHECK(Util.is_upper()); \
	CHECK(Util.is_lower()); \
	CHECK(EU * U == Util * EU); \
}


TEST_CASE_TEMPLATE("Commutation Relations", T, F2, F3, F5, Q) {
	using VT = SparseVector<T, size_t>;
	using MatT = ColumnMatrix<VT>;

	SUBCASE("Square") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			size_t m = 20;

			CHECK_EL_L(m, m, 0.04, 0.1, generator)
			CHECK_L_EL(m, m, 0.04, 0.1, generator)
			CHECK_U_EU(m, m, 0.04, 0.1, generator)
			CHECK_EU_U(m, m, 0.04, 0.1, generator)
		}
	}

	SUBCASE("Short") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			size_t m = 20;
			size_t n = 30;

			CHECK_EL_L(m, n, 0.03, 0.1, generator)
			CHECK_L_EL(m, n, 0.03, 0.1, generator)
			CHECK_U_EU(m, n, 0.03, 0.1, generator)
			CHECK_EU_U(m, n, 0.03, 0.1, generator)

		}
	}

	SUBCASE("Tall") {
		for ( unsigned seed = 0; seed < N_SEEDS; seed++) {
			std::default_random_engine generator(seed);
			size_t m = 30;
			size_t n = 20;

			CHECK_EL_L(m, n, 0.03, 0.1, generator)
			CHECK_L_EL(m, n, 0.03, 0.1, generator)
			CHECK_U_EU(m, n, 0.03, 0.1, generator)
			CHECK_EU_U(m, n, 0.03, 0.1, generator)
		}
	}

}
