#pragma once

/*
An implementation of a sparse vector
maintains indices in sorted order
*/


#include <cstddef>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <utility>
#include <random>
#include <chrono>
#include <map>
#include "field.hpp"
#include "abstract_vector.hpp"


// template over type of values
template <typename TV, typename TI=size_t>
class SparseVector
{
private:

	using key_type = nzpair<TI, TV>; // std::pair<TI, TV>

	std::vector<key_type> indval;

public:
	// sort in-place
	void sort() {
		// sort in-place
		if (indval.empty()) { return; } // nothing to do
		std::sort(indval.begin(), indval.end());
		// implement a simple bubble sort to avoid overhead of std::sort
		// size_t max_swap = indval.size()-1;
		// while (max_swap > 0) {
		// 	size_t cur_max = max_swap;
		// 	max_swap = 0;
		// 	for (size_t i = 0; i < cur_max; i++) {
		// 		if (indval[i+1] < indval[i]) {
		// 			max_swap = i;
		// 			// swap values
		// 			std::swap(indval[i], indval[i+1]);
		// 		}
		// 	}
		// }
	}

private:
	void check_sorted() const {
		if (indval.size() < 2) { return; }
		for (auto it = indval.begin(); it != --(indval.end()); ) {
			if (!(it->ind < (++it)->ind)) { throw std::runtime_error("SparseVector not sorted!"); }
		}
	}

public:
	using val_type = TV;
	using tmp_type = std::vector<key_type>;

	SparseVector() {}

	SparseVector(const std::vector<key_type> &indval) : indval(indval) {
		#ifdef BATS_DEBUG
		check_sorted();
		#endif
	}

	SparseVector(const std::vector<TI> &ind, const std::vector<TV> &val) {
		size_t nz = ind.size();
		indval.reserve(nz);
		for (size_t i = 0; i < nz; i++) {
			indval.emplace_back(key_type(ind[i], val[i]));
		}
		#ifdef BATS_DEBUG
		check_sorted();
		#endif
	}

	// constructor from other sparse vector
	template<typename TI2>
	SparseVector(const SparseVector<int, TI2> &other) {
		indval.reserve(other.nnz());
		for (auto it = other.nzbegin(); it < other.nzend(); ++it) {
			indval.emplace_back(key_type((*it).ind, (*it).val));
		}
		#ifdef BATS_DEBUG
		check_sorted();
		#endif
	}

	// constructor from tuples of indices and int values
	SparseVector(const std::vector<std::tuple<size_t, int>>& ival) {
		indval.reserve(ival.size());
		for (auto& tup : ival) {
			indval.emplace_back(key_type(
				std::get<0>(tup),
				std::get<1>(tup)
			));
		}
		sort();
	}

	// constructor that loops over index and value iterators
	// can be iterators over different type
	template <typename IT1, typename IT2>
	SparseVector(IT1 indit, IT2 valit, size_t n) {
		indval.reserve(n);
		for (size_t i = 0; i < n; i++) {
			indval.emplace_back(key_type(TI(*indit++), TV(*valit++)));
		}
		#ifdef BATS_DEBUG
		check_sorted();
		#endif
	}


	// cosntructor that returns indicator in given index
	SparseVector(const TI i, const TV v=TV(1)) {
		indval.push_back(key_type(i, v));
	}

	// construct from line string
	SparseVector(std::string &line) {
		std::string token;
		std::istringstream iss(line);
		while (getline(iss, token, ',')) {
			// std::cout << token << ',' << std::endl;
			if (token.size() > 0) {
				indval.emplace_back(key_type(token));
			}
		}
	}

	// return new SparseVector where all indices have been shifted
	SparseVector shift_inds(TI shift) const {
		SparseVector v(*this);
		for (auto& x : v.indval) {
			x.ind += shift;
		}
		return v;
	}



	// SparseVector& operator=(const SparseVector &other) {
	// 	indval = other.indval;
	// 	return *this;
	// }

	template<typename T>
	bool operator==(const T &other) const {
		auto it1 = nzbegin();
		auto it2 = other.nzbegin();
		while (it1 != nzend() && it2 != other.nzend()) {
			if (*it1++ != *it2++) {return false;}
		}
		if (it1 != nzend() || it2 != other.nzend()) { return false;}
		return true;
	}
	template<typename T>
	inline bool operator!=(const T &other) const {
		return !(*this == other);
	}


	// extract indices by iterator
	template<typename T>
	SparseVector operator[](const T &indset) const {
		auto it = indset.cbegin();
		auto end = indset.cend();
		std::vector<key_type> indval2;
		indval2.reserve(indval.size());
		auto selfit = indval.cbegin();
		auto selfend = indval.cend();
		size_t ct = 0;
		while (it != end && selfit != selfend) {
			if (*it < selfit->ind) {
				// zero entry
				// indval2.emplace_back(key_type(ct, TV(0)));
				++it;
				++ct;
			} else if (*it == selfit->ind) {
				indval2.emplace_back(key_type(ct, selfit->val));
				++selfit;
				++it;
				++ct;
			} else {
				++selfit;
			}
		}
		// only enter this loop if index set is longer than nonzeros
		// do not want to put zero values in
		// while (it != end) {
		// 	indval2.emplace_back(key_type(ct, TV(0)));
		// 	++it;
		// 	++ct;
		// }
		return SparseVector(indval2);
	}

	// get const iterator through nzs
	inline auto nzbegin() const { return indval.cbegin(); }
	inline auto nzend() const { return indval.cend(); }
	// get nonconst iterator through nzs
	inline auto nzbegin() { return indval.begin(); }
	inline auto nzend(){ return indval.end(); }

	inline void clear() {indval.clear();}
	// clear and release memory by swapping with empty vector.
	inline void clear_dealloc() { std::vector<key_type>().swap(indval); }

	void clear_zeros() {
		std::vector<key_type> indval2;
		for (auto iv : indval) {
			if (iv.val != TV(0)) {
				indval2.emplace_back(iv);
			}
		}
		indval = indval2;
	}

	/**
	clear indices marked true in c
	i.e. remove entry i if c[i] is true
	*/
	void clear_inds(const std::vector<bool> &c) {
		std::vector<key_type> indval2;
		for (auto iv : indval) {
			// put indval pair in if it is not marked for clearing
			if (!c[iv.ind]) {
				indval2.emplace_back(iv);
			}
		}
		indval = indval2;
	}
	// use temporary vector
	void clear_inds(
		const std::vector<bool> &c,
		std::vector<key_type>& tmp
	) {
		tmp.clear(); // clear temporary vector
		for (auto iv : indval) {
			// put indval pair in if it is not marked for clearing
			if (!c[iv.ind]) {
				tmp.emplace_back(iv);
			}
		}
		std::swap(indval, tmp);
	}

	// nnz
	inline size_t nnz() const {return indval.size(); }

	std::vector<size_t> nzinds() const {
		std::vector<size_t> ind;
		ind.reserve(nnz());
		for (auto it = nzbegin(); it != nzend(); it++) {
			ind.emplace_back(it->ind);
		}
		return ind;
	}

	std::vector<TV> nzvals() const {
		std::vector<TV> val;
		val.reserve(nnz());
		for (auto it = nzbegin(); it != nzend(); it++) {
			val.emplace_back(it->val);
		}
		return val;
	}

	std::tuple<std::vector<size_t>, std::vector<TV>> nzs() const {

		std::vector<size_t> ind; ind.reserve(nnz());
		std::vector<TV> val; val.reserve(nnz());
		for (auto it = nzbegin(); it != nzend(); it++) {
			ind.emplace_back(it->ind);
			val.emplace_back(it->val);
		}

		return std::make_tuple(ind, val);
	}

	// returns iterator pointing to first element that is not less than i
	inline auto lower_bound(const TI &i) {
		return std::lower_bound(nzbegin(), nzend(), key_type(i, TV(0)));
	}
	inline auto lower_bound(const TI &i) const {
		return std::lower_bound(nzbegin(), nzend(), key_type(i, TV(0)));
	}
	// returns iterator pointing to first element that is greater than i
	inline auto upper_bound(const TI &i) {
		return std::upper_bound(nzbegin(), nzend(), key_type(i, TV(0)));
	}
	inline auto upper_bound(const TI &i) const {
		return std::upper_bound(nzbegin(), nzend(), key_type(i, TV(0)));
	}

	// replace the item at the iterator location with k
	template <typename itT>
	auto replace(itT &it, const key_type &k) {
		*it = k;
		return it;
	}
	template <typename itT>
	inline auto replace(itT &it, const TI ind, const TV val) { return replace(it, key_type(ind, val));}
	template <typename itT>
	inline auto replace(itT &it, const TV val) { return replace(it, key_type((*it).ind, val));}

	TV getval(const size_t i) const {
		auto it = lower_bound(i);
		if (it == nzend() || (*it).ind != i) {
			return TV(0);
		}
		return (*it).val;
	}

	TV operator[](size_t i) const {
		return getval(i);
	}

	inline auto emplace_back(const key_type &k) {
		return indval.emplace_back(k);
	}
	inline auto emplace_back(TI ind, TV val) {
		return indval.emplace_back(key_type(ind, val));
	}

	// find nonzero index of last element with index < i
	auto find_last_nz(TI i) {
		auto it = std::lower_bound(
			indval.cbegin(),
			indval.cend(),
			key_type(i, TV(0))
		);
		// if there is no element with index < i, return pointer to end
		return it == indval.cend() ? it : --it;
		//return it--;
	}


	// return last nonzero
	inline const key_type& lastnz() const {
		return indval.back();
	}

	inline const key_type& firstnz() const {
		return indval[0];
	}


	// permute in-place
	// O(nz log nz) where nz is #non-zeros
	// index i maps to perm[i]
	// i.e. [2, 0, 1] applied to indval(0, 1) will map to indval(2,1)
	void permute(const std::vector<size_t>  &perm) {
		for (size_t i = 0; i < nnz(); i++) {
			indval[i].ind = perm[indval[i].ind];
		}
		sort();
	}

	// apply inverse permutation in-place
	// O(nz log nz) where nz is #non-zeros
	void ipermute(const std::vector<size_t>  &perm) {
		for (size_t i = 0; i < nnz(); i++) {
			indval[i].ind = perm[indval[i].ind];
		}
		sort();
	}

	// return subvector based on indices pind
	SparseVector subvector(
		const std::vector<size_t> &pind
	) const {
		std::vector<key_type> newindval;
		for (auto it = nzbegin(); it != nzend(); it++) {
			if (pind[it->ind] != bats::NO_IND) {
				newindval.emplace_back(key_type(pind[it->ind], it->val));
			}
		}
		// sort in-place
		std::sort(newindval.begin(), newindval.end());
		return SparseVector(newindval);
	}

	// block v[i0:i1]
	// i1 is not inclusive
	// new indices start at 0.
	SparseVector block(
		const size_t i0,
		const size_t i1
	) const {
		std::vector<key_type> newindval;
		auto it = lower_bound(i0);
		while( it != nzend() ) {
			if (it->ind < i1) {
				newindval.emplace_back(key_type(it->ind - i0, it->val));
			} else {
				break;
			}
			it++;
		}
		return SparseVector(newindval);
	}

	// set v[i0:i1] = b
	void set_block(
		const size_t i0,
		const size_t i1,
		const SparseVector& b
	) {

		// first delete all entries in the range
		indval.erase(lower_bound(i0), upper_bound(i1));

		// now insert the elements in b
		auto it = lower_bound(i0);
		auto bit = b.nzbegin();
		while (bit != b.nzend()) {
			it = indval.emplace(it, key_type(bit->ind + i0, bit->val));
			bit++; it++;
		}
	}

	// apply J-matrix transformation
	// index i -> (m-1) - i
	void J(const size_t m) {
		for (size_t i = 0; i < nnz(); i++) {
			indval[i].ind = (m - 1) - indval[i].ind;
		}
		sort();
	}

	/*
	Row operations
	*/
	// swap entries i and j in-place
	void swap_rows(const size_t i, const size_t j) {
		auto pi = lower_bound(i);
		auto pj = lower_bound(j);
		// check whether there is actually an entry at location i
		if (pi != nzend() && pi->ind == i) {
			// check whether there is actually an entry at location j
			if (pj != nzend() && pj->ind == j) {
				std::swap(pi->val, pj->val);
			} else {
				pi->ind = j;
				sort();
			}
		} else if (pj != nzend() && pj->ind == j) {
			pj->ind = i;
			sort();
		}
	}
	// scale vector by c
	void scale_inplace(const TI i, const TV& c) {
		auto pi = lower_bound(i);
		if (pi != nzend() && pi->ind ==i) {
			if (c == 0) {
				indval.erase(pi);
				return;
			}
			pi->val = c * pi->val;
		}
		return;
	}

	//deletetion of the last row of a column of a matrix
	//the index i is provided to check if i is the index of
	//the last the non-zero elements
	void erase_last_row_of_matrix(const TI i){
		// first check if it is an empty vector!!!!!!
		if(nzend() != nzbegin()){
			// if the last non-zero term's index is i
			if(i == (indval.end()-1)->ind){
				indval.pop_back();
			}
		}
	}

	//deletetion of row i , the indices after i should minus one
	void erase_for_matrix(const TI i){
		// if it is an empty vector, do nothing
		if (nzend() == nzbegin()) return;

		auto pi = lower_bound(i);
		if (pi != nzend()) {
			// if the i-th row is non-zero (or find it)
			if (pi->ind ==i){
				indval.erase(pi);
			}
			// decrement the indices after i
			while(pi < nzend()){
				(pi->ind)--;
				pi++;
			}
		}
	}

	// mix row entries
	// apply matrix
	// [a b]
	// [c d]
	// to rows i, j
	// v[i] <- a*v[i] + b*v[j]
	// v[j] <- c*v[i] + d*v[j]
	// check that zeros are removed
	void mix_rows(const size_t i, const size_t j, const TV& a, const TV& b, const TV& c, const TV& d) {
		auto pi = lower_bound(i);
		auto pj = lower_bound(j);
		// check whether there is actually an entry at location i
		if (pi != nzend() && pi->ind == i) {
			auto vi = pi->val;
			// check whether there is actually an entry at location j
			if (pj != nzend() && pj->ind == j) {
				auto vj = pj->val;
				pi->val = (a * vi) + (b * vj);
				pj->val = (c * vi) + (d * vj);
				if (pj->val ==  0) {
					indval.erase(pj);
					pi = lower_bound(i);
				}
			} else {
				pi->val = a * vi;
				if (c*vi != 0) {
					indval.insert(pj, nzpair(j, c*vi));
					pi = lower_bound(i);
				}
			}

			if (pi->val == 0) {
				indval.erase(pi);
			}
		} else if (pj != nzend() && pj->ind == j) {
			auto vj = pj->val;

			if (d*vj != 0) {
				pj->val = d * vj;
			} else {
				indval.erase(pj);
			}
			if (b*vj != 0) indval.insert(pi, nzpair(i, b*vj));

		}
	}

	/**
	insert row indices at specified locations
	assumes that list of rows is in sorted order
	*/
	void insert_rows(const std::vector<size_t>& r_inds) {
		// size_t offset = 0;
		if(r_inds.begin()!=r_inds.end()){ //non-empty check 
			auto iv = indval.begin();
			while (iv != indval.end()) {
				// search for how many rows will be inserted
				auto ri = std::lower_bound(r_inds.begin(), r_inds.end(), iv->ind);
				iv->ind += std::distance(r_inds.begin(), ri);

				// insert additional rows as necessary
				while (*ri <= iv->ind && ri != r_inds.end()) {++ri; iv->ind++;}

				++iv;
			}
			// // we only enter this loop if we already inserted all rows.
			// while (iv != indval.end()) {
			// 	std::cout << "\ngo into the second loop of insert rows" << std::endl;
			// 	iv->ind += r_inds.size();
			// 	++iv;
			// }
		}
		
	}

	// // v[i] <- v[i] + c * v[j]
	// void add_rows(const size_t i, const TV& c, const size_t j) {
	// 	if (c == 0) return;
	// 	auto pi = lower_bound(i);
	// 	if (pi != nzend() && pi->ind == i) {
	// 		// something to do
	// 		auto pj = lower_bound(j);
	// 		if (pj != nzend() && pj->ind ==j) {
	// 			pj->val = pj->val + c * (pi->val);
	// 			if (pj->val == 0) indval.erase(pj); // delete entry if value is 0
	// 		} else {
	// 			// need to insert
	// 			indval.insert(pj, c*(pi->val));
	// 		}
	// 	}
	// }

	/**
	calculate the number of non-zeros common to two vectors

	@param x sparse vector for comparison
	@return ct number of non-zeros common to both this vector and x
	*/
	template <class SVT>
	size_t nnz_intersection(
		const SVT& x
	) const {

		if (x.nnz() == 0) {return 0;}

		size_t ct = 0;
		auto i1 = nzbegin();
		auto i2 = x.nzbegin();

		while (i1 != nzend() && i2 != x.nzend()) {
			if (i1->ind == i2->ind) {
				++ct;
				++i1;
				++i2;
			} else if (i1->ind < i2->ind) {
				++i1;
			} else { // i2->ind < i1->ind
				++i2;
			}
		}
		return ct;
	}

	/**
	calculate the number of non-zeros common with c * x
	where c ranges over any possible value

	@param x sparse vector for comparison
	@param ct map from coefficient c to number of identical nonzeros with c*x
	ct is not cleared in the function.
	*/
	template <class SVT>
	void coeff_intersection(
		const SVT& x,
		std::map<TV, size_t>& ct
	) const {

		auto i1 = nzbegin();
		auto i2 = x.nzbegin();

		while (i1 != nzend() && i2 != x.nzend()) {
			if (i1->ind == i2->ind) {
				auto c = i1->val / i2->val;
				++(ct[c]);
				++i1;
				++i2;
			} else if (i1->ind < i2->ind) {
				++i1;
			} else { // i2->ind < i1->ind
				++i2;
			}
		}
		return;
	}

	// set
	// y <- ax + y
	// uses pre-allocated temporary vector
	template <class SVT>
	void axpy(
		const TV& a,
		const SVT& x,
		std::vector<key_type>& tmp
	) {

		if (a == TV(0)) {clear_zeros(); return;}

		// set i2 to find first ind >= firstind
		auto i2 = x.nzbegin();
		// where to put new vector
		if (i2 == x.nzend()) { return; } // nothing to do
		// something to do...
		auto i1 = indval.cbegin();
		tmp.clear();
		while (i1 != indval.cend() && i2 != x.nzend()) {
			if ( i1->ind == i2->ind) {
				TV val = (a * ((*i2).val)) + (*i1).val;
				// std::cout << "a: " << a << " x: " << ((*i2).val)) << " y: " << (*i1).val << std::endl;
				if (val != TV(0)) {
					tmp.emplace_back(key_type(i1->ind, val));
				}
				++i1;
				++i2;
			} else if ((i1->ind) < (i2->ind)) {
				tmp.emplace_back(*i1++);
				// ++i1;
			} else {
				tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
				++i2;
			}
		}
		// run through rest of entries and dump in
		// at most one of the loops does anything
		while (i1 != indval.cend()) {
			tmp.emplace_back(*i1++);
			// ++i1;
		}
		while (i2 != x.nzend()) {
			tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
			++i2;
		}

		// copy temp vector to indval
		// indval.resize(tmp.size());
		// std::copy(tmp.cbegin(), tmp.cend(), indval.begin());
		std::swap(indval, tmp);
		// indval = tmp;

		//clear_zeros();

		return;
	}


	// set
	// y <- ax + y
	template <class SVT>
	void axpy(
		const TV &a,
		const SVT &x
	) {

		if (a == TV(0)) {clear_zeros(); return;}

		// set i2 to find first ind >= firstind
		auto i2 = x.nzbegin();
		// where to put new vector
		if (i2 == x.nzend()) { return; } // nothing to do
		// something to do...
		auto i1 = indval.cbegin();
		std::vector<key_type> tmp;
		while (i1 != indval.cend() && i2 != x.nzend()) {
			if ( i1->ind == i2->ind) {
				TV val = (a * ((*i2).val)) + (*i1).val;
				// std::cout << "a: " << a << " x: " << ((*i2).val)) << " y: " << (*i1).val << std::endl;
				if (val != TV(0)) {
					tmp.emplace_back(key_type(i1->ind, val));
				}
				++i1;
				++i2;
			} else if ((i1->ind) < (i2->ind)) {
				tmp.emplace_back(*i1);
				++i1;
			} else {
				tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
				++i2;
			}
		}
		// run through rest of entries and dump in
		// at most one of the loops does anything
		while (i1 != indval.cend()) {
			tmp.emplace_back(*i1);
			++i1;
		}
		while (i2 != x.nzend()) {
			tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
			++i2;
		}

		// copy temp vector to indval
		// indval.resize(tmp.size());
		// std::copy(tmp.cbegin(), tmp.cend(), indval.begin());
		std::swap(indval, tmp);
		// indval = tmp;

		#ifdef BATS_DEBUG
		check_sorted();
		#endif

		//clear_zeros();

		return;
	}

	SparseVector operator+(const SparseVector& other) const {
		SparseVector a(*this);
		a.axpy(TV(1), other);
		return a;
	}

	// scalar multiplication
	SparseVector operator*(const TV a) const {
		if (a == 0) { return SparseVector(); }
		SparseVector av(*this);
		for (auto& ival : av.indval) {
			ival.val = ival.val * a;
		}
		return av;
	}

	// return self + ax[firstind:lastind]
	// template over sparse vector type
	template <class SVT>
	void axpy(
		const TV &a,
		const SVT &x,
		const TI &firstind,
		const TI &lastind,
		std::vector<key_type>& tmp
	) {

		if (a == TV(0)) {return;}

		// set i2 to find first ind >= firstind
		auto i2 = std::lower_bound(
			x.nzbegin(),
			x.nzend(),
			key_type(firstind, TV(0))
		);
		// alternative: try just incrementing to firstind
		// auto i2 = x.nzbegin();
		// while (i2 != x.nzend() && i2->ind < firstind) {++i2;}
		// where to put new vector
		if (i2 == x.nzend() || !(i2->ind < lastind)) { return; } // nothing to do
		// something to do...
		auto i1 = indval.cbegin();
		// std::vector<key_type> tmp;
		tmp.clear();
		while (i1 != indval.cend() && i2 != x.nzend() && (i2->ind < lastind)) {
			if (i1->ind == i2->ind) {
				TV val = (a * (i2->val)) + i1->val;
				if (val != TV(0)) {
					tmp.emplace_back(key_type(i1->ind, val));
				}
				++i1;
				++i2;
				// if (!(i2->ind < lastind)) { break; }
			} else if (i1->ind < i2->ind) {
				tmp.emplace_back(*i1);
				++i1;
			} else {
				tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
				++i2;
				// if (!(i2->ind < lastind)) { break; }
			}
		}
		// run through rest of entries and dump in
		// at most one of the loops does anything
		while (i1 != indval.cend()) {
			tmp.emplace_back(*i1);
			++i1;
		}
		while (i2 != x.nzend() && (i2->ind < lastind)) {
			tmp.emplace_back(key_type(i2->ind, a * (i2->val)));
			++i2;
		}
		// swap indval and tmp
		std::swap(indval, tmp);
		return;
	}

	// return self + coeff*x[inds]
	// template over sparse vector type
	template <class SVT>
	void axpy(
		const SVT &x,
		const std::vector<TV> &coeff,
		const std::vector<TI> &inds,
		std::vector<key_type>& tmp
	) {

		// set i2 to find first ind >= firstind
		auto i2 = x.nzbegin(); // iterate over x
		// where to put new vector
		if (i2 == x.nzend()) { return; } // nothing to do
		// something to do...
		auto i1 = indval.cbegin(); // iterate over self
		size_t ii = 0; // index for inds

		// std::vector<key_type> tmp;
		tmp.clear();
		while (i1 != indval.cend() && i2 != x.nzend() && ii < inds.size()) {
			if (ii < i1->ind  && inds[ii] < i2->ind) {
				// need to increment ii until something happens
				++ii;
			} else if (i1->ind == ii && inds[ii] < i2->ind) {
				// no match from nonzero in x, but match from nonzero in self
				tmp.push_back(*i1);
				++i1;
				++ii;
			} else if (ii < i1->ind && inds[ii] == i2->ind) {
				// no match from nonzero in self, but match from x
				TV val = (coeff[ii] * (i2->val));
				if (val != TV(0)) {
					tmp.push_back(key_type(ii, val));
				}
				++ii;
				++i2;
			} else if (i1->ind == ii && i2->ind == inds[ii]) {
				// match from both x and self
				TV val = (coeff[ii] * (i2->val)) + i1->val;
				// std::cout << "a: " << a << " x: " << ((*i2).val)) << " y: " << (*i1).val << std::endl;
				if (val != TV(0)) {
					tmp.push_back(key_type(i1->ind, val));
				}
				++i1;
				++i2;
				++ii;
			} else {
				throw std::runtime_error("Unexpected condition in sparse axpy!");
			}
		}
		// run through rest of entries and dump in
		// at most one of the loops does anything
		while (i1 != indval.cend()) {
			tmp.push_back(*i1);
			++i1;
		}
		while (i2 != x.nzend() && ii < inds.size()) {
			if (inds[ii] < i2->ind) {
				++ii;
			} else {
				tmp.push_back(key_type(ii, coeff[ii] * (i2->val)));
				++i2;
				++ii;
			}
		}

		// copy temp vector to indval
		std::swap(indval, tmp);

		return;
	}

	// scale each entry i by coeff[i]
	// assume coeff[i] is non-zero
	void scale_inplace(const std::vector<TV> &coeff) {
		auto it = nzbegin();
		while (it != nzend()) {
			it->val = (it->val * coeff[it->ind]);
			++it;
		}
	}

	// scale vector by c
	void scale_inplace(const TV c) {
		auto it = nzbegin();
		while (it != nzend()) {
			it->val = it->val * c;
			++it;
		}
	}

	SparseVector scale(const TV c) const {
		SparseVector vc(*this);
		vc.scale_inplace(c);
		return vc;
	}


	void print() const {
		auto it = indval.cbegin();
		while (it != indval.cend()) {
			std::cout << *it << std::endl;
			++it;
		}
	}

	void print_row() const {
		auto it = indval.cbegin();
		while (it != indval.cend()) {
			std::cout << *it << ' ';
			++it;
		}
		std::cout << std::endl;
	}

	template <typename IO>
	void write(IO &io) const {
		auto it = indval.cbegin();
		while (it != indval.cend()) {
			io << (*it).ind << ':' << (*it).val << ',';
			++it;
		}
		io << '\n';
	}

	std::string str() {
	  std::ostringstream oss;
	  write(oss);
	  return oss.str();
	}

	// generate random vectors
	static SparseVector random(size_t n, double p, int maxval, std::default_random_engine &generator) {
		std::uniform_int_distribution<int> val_distribution(1,maxval);
		std::uniform_real_distribution<double> nz_distribution(0.0,1.0);

		std::vector<key_type> indval;
		for (size_t i = 0; i < n; i++) {
			if (nz_distribution(generator) < p) {
				indval.emplace_back(key_type(i, val_distribution(generator)));
			}
		}
		return SparseVector(indval);
	}
	static SparseVector random(size_t n, double p, int maxval) {
		// obtain a seed from the system clock:
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine generator(seed);
		return random(n, p, maxval, generator);
	}

	// tensor product a \otimes b
	// m is length of vector b (other)
	// uses Kronecker product ordering
	SparseVector kron(const SparseVector& other, size_t m) const {
		std::vector<key_type> nzs;
		auto Ait = nzbegin();
		while (Ait != nzend()) {
			auto Bit = other.nzbegin();
			while (Bit != other.nzend()) {
				size_t ind = m*(Ait->ind) + Bit->ind;
				TV val = (Ait->val) * Bit->val;
				nzs.emplace_back(nzpair(ind, val));
				Bit++;
			}
			Ait++;
		}
		return SparseVector(nzs);
	}
	inline SparseVector tensor(const SparseVector& other, size_t m) const {return tensor(other, m);}
};
