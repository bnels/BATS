

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <linalg/naive_dense.h>
#include <linalg/field.h>

#define F3 ModP<int, 3>


TEST_CASE("Matrix Multiplication") {

	int a1v[] = {
	2,3,4,
	1,2,3,
	8,5,2,
	};
	A<Dense<int>> a1(3,3,a1v);

	int a2v[] = {
	39,32,25,
	28,22,16,
	37,44,51,
	};
	A<Dense<int>> a2(3,3,a2v);

	CHECK( (matmul(a1,a1) == a2) );
}


TEST_CASE("ColView") {
	using AD = A<Dense<F3>>;
	srand(0);

	AD a1(4,4);
	AD a2(4,4);
	fill_rand(a2);
	a1[2]=a2[3];

	F3 resv[] = {
	0,0,0,0,
	0,0,0,0,
	1,0,1,1,
	0,0,0,0,
	};
	AD res(4,4,resv);

	CHECK( (a1==res) );
}


TEST_CASE("RowView") {
	using AD = A<Dense<F3>>;
	srand(0);

	AD a1(4,4);
	AD a2(4,4);
	fill_rand(a2);
	a1.r(2)=a2.r(3);

	F3 resv[] = {
	0,0,2,0,
	0,0,1,0,
	0,0,2,0,
	0,0,1,0,
	};
	AD res(4,4,resv);

	CHECK( (a1==res) );
}

TEST_CASE("el_commute") {
	int elmat1v[] = {1,0,0,0, 
		          0,1,0,0,
		          0,0,0,1,
		          0,0,0,0};
	EL<Dense<int>> elmat1(4,4,elmat1v);

	int lmat1v[] = {2,2,3,4, 
		          0,3,2,4,
		          0,0,4,4,
		          0,0,0,5};
	L<Dense<int>> lmat1(4,4,lmat1v);

	auto res = el_commute(elmat1,lmat1);

	CHECK( (matmul(res,elmat1)== matmul(elmat1,lmat1)) );
}
