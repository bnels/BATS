#pragma once
/*
Cellular map f: X->Y
data is how cells in X map to cells in Y
essentially a chain map
*/

#include <vector>
#include <string>
#include <linalg/sparse_vector.hpp>
#include <linalg/col_matrix.hpp>

namespace bats {

class CellularMap
{
private:
	using vec_type = SparseVector<int, size_t>;
	using map_type = ColumnMatrix<vec_type>;

	// store image of k-cells as linear combination of k-cells
	std::vector<map_type> cell_map;

	inline void _resize(size_t dim) { cell_map.resize(dim); }
	inline void _safe_resize(size_t dim) { if (dim >= cell_map.size()) { _resize(dim); } }

public:

	CellularMap() {}

	// allocate for dim dimensions
	CellularMap(size_t dim) {
		cell_map.resize(dim+1);
	}

	CellularMap(std::string &fname) {
		std::ifstream file (fname, std::ios::in);
        if (file.is_open()) {
			std::string line;
			getline(file, line); // TODO: should be CellularMap
			// keep putting cell_maps on back
			while (!file.eof()) {
				cell_map.emplace_back(map_type(file));
			}
            file.close();
        } else {
            std::cerr << "unable to read CellularMap from " << fname << std::endl;
        }
	}

	inline size_t maxdim() const { return cell_map.size() - 1; }

	map_type& operator[](size_t k) {
		_safe_resize(k);
		return cell_map[k];
	}

	inline const map_type& operator[](size_t k) const {
		return cell_map[k];
	}

	// identity map
	// template over complex type
	template <typename CpxT>
	static CellularMap identity(const CpxT &X) {
		size_t maxk = X.maxdim();
		CellularMap M(maxk);
		for (size_t k = 0; k < maxk + 1; k++) {
			M[k] = map_type::identity(X.ncells(k));
		}
		return M;
	}

	void save(std::string &fname) const {
		// save each cellmap in the same file
		std::ofstream file (fname, std::ios::out);
        if (file.is_open()) {
			file << "CellularMap\n";
			for (auto& M : cell_map) {
				M.write(file);
			}
            file.close();
        } else {
            std::cerr << "unable to write CellularMap to " << fname << std::endl;
        }
	}

};

} // namespace bats
