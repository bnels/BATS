#pragma once
/*
Sparse factorizations of ColumnMatrix

LEUP, PUEL, UELP, PLEU

as well as shape commutation relations
*/

#include "col_matrix.h"
#include <vector>
#include <map>


// factorization struct
template <class TC>
struct SparseFact {
    // struct to hold factorization LEUP, PUEL, etc.
    ColumnMatrix<TC> L;
    ColumnMatrix<TC> E;
    ColumnMatrix<TC> U;
    ColumnMatrix<TC> P;

    inline ColumnMatrix<TC> LEUP_prod() const {
        return L * E * U * P;
    }

    inline ColumnMatrix<TC> PLEU_prod() const {
        return P * L * E * U;
    }

    inline ColumnMatrix<TC> UELP_prod() const {
        return U * E * L * P;
    }

    inline ColumnMatrix<TC> PUEL_prod() const {
        return P * U * E * L;
    }


};

template <class TC>
inline void update_pivot(const ColumnMatrix<TC> &A, std::map<size_t, std::vector<size_t>> &p2c, size_t j, size_t i0) {
    // update pivot for column j, looking past row i0
    auto piv = A[j].lower_bound(i0);
    if (piv != A[j].nzend()) {p2c[piv->ind].emplace_back(j);}
}

template <class TC>
inline void delete_pivot(const ColumnMatrix<TC> &A, std::map<size_t, std::vector<size_t>> &p2c, size_t j, size_t i0) {
    // update pivot for column j, looking past row i0
    auto piv = A[j].lower_bound(i0);
    if (piv != A[j].nzend()) {
        for (auto it = p2c[piv->ind].begin(); it != p2c[piv->ind].end(); ++it) {
            if (*it == j) { p2c[piv->ind].erase(it); break; }
        }
    }
}

template <class TC>
std::map<size_t, std::vector<size_t>> get_pivots(const ColumnMatrix<TC> &A) {
    std::map<size_t, std::vector<size_t>> piv2cols;
    for (size_t j = 0; j < A.ncol(); j++) {
        update_pivot(A, piv2cols, j, 0);
    }
    return piv2cols;
}

template <class TC>
void LEUP_inplace(SparseFact<TC> &F) {
    // first do LEUP in-place on E matrix
    // we'll operate on the transpose of P to keep track of permutations

    using val_type = typename TC::val_type;

    std::vector<size_t> pivs; // records pivots
    std::vector<val_type> coeff; // records coefficients

    auto p2c = get_pivots(F.E);
    size_t m = F.E.nrow();
    size_t n = F.E.ncol();

    size_t i = 0;
    size_t j = 0;
    while (i < m && j < n) {

        if (p2c.count(i) > 0) {

            // get pivot column
            size_t j2 = p2c[i][0];

            if (j2 != j) {
                // delete previous pivot for j
                delete_pivot(F.E, p2c, j, i);

                // swap with column j
                F.E.swap_cols(j, j2);
                // update pivot for j2 since we swapped
                update_pivot(F.E, p2c, j2, i); // update pivot

                // apply to permutation as well
                F.P.swap_cols(j, j2);
            }

            // lazy update U factor
            F.U[j].axpy(F.E[j], coeff, pivs);


            // perform schur complement in lower right-hand block
            auto a11 = F.E(i, j);

            // we pivot on row i
            pivs.emplace_back(i);
            coeff.emplace_back(a11.inv()); // store for later

            // loop over columns with this pivot starting with second entry
            for (auto jj = ++(p2c[i].cbegin()); jj < p2c[i].cend(); jj++) {
                   auto c = F.E(i, *jj) / a11;
                   F.E[*jj].axpy(-c, F.E[j], i+1, m); // update block
                   update_pivot(F.E, p2c, *jj, i+1); // update pivot
            }
            p2c.erase(i); // clear out old data

            F.L[i].axpy(a11.inv(), F.E[j], i+1, m);


            // clear out column
            F.E[j] = TC();
            F.E[j].emplace_back(i, a11);

            i++;
            j++;
        } else {
            i++;
        }
    }
    // now finish lazy updates of U factor
    while (j < n) {
        // lazy update U factor
        F.U[j].axpy(F.E[j], coeff, pivs);
        // Clear out column of E
        F.E[j] = TC();
        j++;
    }

    // transpose the permutation matrix P
    F.P = F.P.transpose();

}

template <class TC>
SparseFact<TC> LEUP(const ColumnMatrix<TC> &A) {
    // LEUP factorization of matrix A

    using MatT = ColumnMatrix<TC>;

    size_t m = A.nrow();
    size_t n = A.ncol();

    SparseFact<TC> F;
    F.L = MatT::identity(m);
    F.E = A;
    F.U = MatT::identity(n);
    F.P = MatT::identity(n);

    LEUP_inplace(F);

    return F;
}

template <class TC>
void PLEU_inplace(SparseFact<TC> &F) {

    F.E = F.E.T(); // take transpose

    LEUP_inplace(F);

    // take transposes of everything
    F.E = F.E.T();
    F.P = F.P.T();
    std::swap(F.L, F.U); // because terms become transposed
    F.U = F.U.T();
    F.L = F.L.T();

}

template <class TC>
SparseFact<TC> PLEU(const ColumnMatrix<TC> &A) {
    // LEUP factorization of matrix A

    auto F = LEUP(A.T());

    // take transposes of everything
    F.E = F.E.T();
    F.P = F.P.T();
    std::swap(F.L, F.U); // because terms become transposed
    F.U = F.U.T();
    F.L = F.L.T();

    return F;
}

template <class TC>
void UELP_inplace(SparseFact<TC> &F) {

    F.E.J_conjugation_inplace(); // conjugation

    LEUP_inplace(F);

    // take J conjugates of everything
    F.E.J_conjugation_inplace();
    F.P.J_conjugation_inplace();
    std::swap(F.L, F.U); // because terms swap triangular shape
    F.U.J_conjugation_inplace();
    F.L.J_conjugation_inplace();

}

template <class TC>
SparseFact<TC> UELP(const ColumnMatrix<TC> &A) {
    // LEUP factorization of matrix A

    auto F = LEUP(A.J_conjugation());

    // take J conjugates of everything
    F.E.J_conjugation_inplace();
    F.P.J_conjugation_inplace();
    std::swap(F.L, F.U); // because terms swap triangular shape
    F.U.J_conjugation_inplace();
    F.L.J_conjugation_inplace();

    return F;
}

template <class TC>
void PUEL_inplace(SparseFact<TC> &F) {

    F.E = F.E.J_conjugation_inplace().T(); // conjugation and transpose

    LEUP_inplace(F);

    // take transposes and J conjugates of everything
    F.E = F.E.J_conjugation_inplace().T();
    F.P = F.P.J_conjugation_inplace().T();
    // don't need to swap
    F.U = F.U.J_conjugation_inplace().T();
    F.L = F.L.J_conjugation_inplace().T();

}

template <class TC>
SparseFact<TC> PUEL(const ColumnMatrix<TC> &A) {
    // LEUP factorization of matrix A

    auto F = LEUP(A.J_conjugation().T());

    // take transposes and J conjugates of everything
    F.E = F.E.T().J_conjugation();
    F.P = F.P.T().J_conjugation();
    // don't need to swap
    F.U = F.U.T().J_conjugation();
    F.L = F.L.T().J_conjugation();

    return F;
}

/*
EL commutation and related functions
*/


// return pivot of column j in E matrix
template <typename TC>
inline size_t pivot_ind(const ColumnMatrix<TC> &E, size_t j) {
    auto it = E[j].nzbegin();
    return (it == E[j].nzend()) ? bats::NO_IND : it->ind;
}

// normalize EL matrix to have unit entries
// return scaling of rows to recover original matrix
template <typename TC>
auto extract_row_scale(ColumnMatrix<TC> &E) {
    // assume EL structure in E
    using val_type = typename TC::val_type;

    size_t m = E.nrow();
    size_t n = E.ncol();

    std::vector<val_type> coeff(m);
    size_t i = 0;
    size_t j = 0;
    while (i < m && j < n) {
        auto it = E[j].nzbegin();
        if (it == E[j].nzend()) {
            // we've reached the end of the non-zero columns
            while (i < m) {
                coeff[i++] = val_type(1);
            }
            break;
        }
        while (i < it->ind) {
            // fill in coefficients for rows with no pivots
            coeff[i++] = val_type(1);
        }
        // set coefficient
        coeff[i++] = it->val;
        // set pivot to 1 in E matrix
        it->val = val_type(1);
        j++;
    }
    while (i < m) {
        // we only enter this loop if E has full column rank, and is tall/skinny
        coeff[i++] = val_type(1);
    }


    return coeff;
}

// produce matrix Ltilde so Ltilde * EL = EL * L
template <typename TC>
ColumnMatrix<TC> EL_L_commute(const ColumnMatrix<TC> &E, const ColumnMatrix<TC> &L) {

    using MatT = ColumnMatrix<TC>;

    MatT EL(E); // create copy
    auto coeff = extract_row_scale(EL); // extract row scale
    // for unit EL, we have EL * L = Ltil * EL
    // with diagonal row scaling, use D EL L = D Ltil EL


    size_t m = EL.nrow();
    size_t n = EL.ncol();

    // first build map of indices
    std::vector<size_t> idx_map(n);
    for (size_t j = 0; j < n; j++) {
        idx_map[j] = pivot_ind(EL, j);
    }

    auto Ltilde = MatT::identity(m);
    // loop over columns of EL
    for (size_t ell = 0; ell < n; ell++) {
        size_t j_ell = idx_map[ell];
        if (j_ell == bats::NO_IND) { break; } // EL structure puts NO_INDS at end
        Ltilde[j_ell].clear(); // clear contents
        for (auto it = L[ell].nzbegin(); it != L[ell].nzend() && idx_map[it->ind] != bats::NO_IND; ++it) {
            Ltilde[j_ell].emplace_back(idx_map[it->ind], it->val); // Ltilde(idx_map[i], idx_map[j]) = L[i,j]
        }
    }

    return Ltilde.row_scale(coeff); // scale rows
}

// produce matrix Ltilde so L * EL = EL * Ltilde
template <typename TC>
inline ColumnMatrix<TC> L_EL_commute(const ColumnMatrix<TC> &L, const ColumnMatrix<TC> &EL) {
    return EL_L_commute(EL.T().J_conjugation_inplace(), L.T().J_conjugation_inplace()).T().J_conjugation_inplace();
}