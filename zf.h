#pragma once
#include <complex>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
//#include <execution>

// Algorithm due to Piotr Kowalczyk,
// "Global Complex Roots and Poles Finding Algorithm Based on Phase Analysis
// for Propagation and Radiation Problems"
// https://arxiv.org/pdf/1806.06522.pdf
// July 25, 2018 pre-print.
//
// This implementation uses a different form of adaptive mesh than the one
// described in the paper. Instead of using Delaunay triangulation to connect
// the midpoint nodes, adding additional nodes and reforming the mesh when
// triangles get too thin, it connects the midpoint nodes and extends the
// subdivided region so that any missing edges are not adjacent to the
// candidate edges.

// To use, just apply the function, solve(), below. The rest is
// implementation, but one could perhaps use the Mesh object directly to
// cache results.
//
// Arguments:
//
// ULcorner, LRcorner : Upper-left and lower-right corners of the 
//		rectangular search region.
// precision: Final length of mesh edges will be less than this.
// f: Any analytic function.
// initial_mesh_len: starting length of mesh edges. Smaller is less likely 
//		to miss a zero, but slower. -1 (default) means the function will
//		try to choose something appropriate.
// edge_limit: If the function ever generates more edges than this number,
// it throws an exception. Default is 50000. Set to 0 to disable and risk 
// letting memory usage explode.
//
// Returns:
// vector of results for each point. first is the location of the zero/pole,
// second is the order. If the order is zero, it may be a regular point, or
// it may signify an equal number of zeros and poles in the same region.
// The algorithm should catch all zeros and poles with a sufficiently fine
// initial mesh, but it cannot know whether it caught every one of them.
// If the it returns fewer zeros or poles than expected, try a finer mesh.
//
// NOTES:
// A limitation of the algorithm is that it does not properly detect zeros
// or poles at branch points or on branch cuts. As a workaround, if you know
// the original function, you may be able to modify it to locate these
// points, e.g. squaring a function with a sqrt() term.
//
// The algorithm's template is designed to accept any "typical" complex
// number type. Specifically, it should implement appropriate overloads for
// the standard math operations, as well as real(), imag(), and arg().
// The underlying data type should support comparison operations, standard 
// math operations, and some common math functions: sin(), atan(), log(), abs(), 
// trunc(). It also requires a specialization of std::hash, which can be defined
// by the user if it doesn't exist. Some of these requirements could be eased
// in the future.
//
// So far the algorithm has been tested with std::complex<double>,
// std::complex<boost::multiprecision::number<boost::multiprecision
// ::cpp_bin_float<100>>, and boost::multiprecision::cpp_complex_quad

namespace zf
{
	template <class cplx> struct get_param;

	template<typename cplx>
	inline std::vector<std::pair<cplx, int>>
		solve(cplx ULcorner, cplx LRcorner,
			typename get_param<cplx>::type precision,
			std::function<cplx(cplx)> f,
			typename get_param<cplx>::type initial_mesh_len
			= typename get_param<cplx>::type(-1.0),
			int edge_limit = 50000);

	template <typename>
	class Mesh;
	template <typename>
	class Edge;
	template <typename>
	class Node;
	template <typename>
	class Triangle;

	// Directions radiating hexagonally from a node.
	enum class Direction
	{
		right = 0,
		up_right,
		up_left,
		left,
		down_left,
		down_right,
	};

	// Left or right side of an edge
	enum class Side
	{
		left = false,
		right = true
	};

	// c++'s built-in mod operator actually gives division-remainder.
	inline int mod(int a, int b)
	{
		a %= b;
		if (a >= 0) return a;
		else return b + a;
	}

	// Extracts the real-part data type of a complex number template class.
	// Assuming the imaginary part will have the same type.
	template <class complex_class>
	struct get_param
	{
		static complex_class C;
		typedef decltype(real(C)) type;
	};

	// Node for a trianglular mesh of complex numbers. Each node can have up to six
	// connections to other nodes, represented by Edges.
	template <typename cplx>
	class Node
	{
	public:
		Node(cplx loc, cplx val);
		cplx location; // Geometric location of node
		int q = 0; // Quadrant of value

		Edge<cplx>* get_edge(int dir);
		void set_edge(Edge<cplx>* e, int dir);
		void calc_q(cplx z);
	private:
		Edge<cplx>* edges[6] = {};
	};

	// Edge for triangular mesh of complex numbers. Each edge connects two
	// nodes. Edges have one of six directions, mainly for facilitating 
	// traversal of the mesh.
	template <typename cplx>
	class Edge
	{
	public:
		Edge(Node<cplx>* node1, Node<cplx>* node2, int dir);
		~Edge();
		Node<cplx>* get_node(int index) const;
		void set_node(int index, Node<cplx>* node);
		Triangle<cplx>* get_tri(int index) const;
		void set_tri(Side index, Triangle<cplx>* tri);
		int get_dir();
		// Returns the clockwise-adjacent edge connected to the specified
		// get_node.
		Edge<cplx>* get_next_CW(int node) const;
		// Returns the counter-clockwise-adjacent edge connected to the 
		// specified get_node.
		Edge<cplx>* get_next_CCW(int node) const;
		// Returns the connected edge in the same direction as this attached to
		// node 1;
		Edge<cplx>* get_continuation();
		// Return the edge "steps" steps counter-clockwise around node;  
		Edge<cplx>* get_next(int node, int steps);

		// Swaps the pointers for node 0 and node 1 and reverses the edge
		// direction. The actual nodes are unaffected.
		void reverse();

		auto length();

		// sets nodes to nullptr without modifying the nodes' stored
		// connections. If any temporary edges are created during intermediate
		// calculations, this must be called before destroying them.
		void detach_nodes();

		int get_dq() const;
		void calc_dq();

		bool boundary = false;
		bool is_new = false;
	private:
		Node<cplx>* nodes[2] = {};
		// Triangles are only needed after adapt_mesh().
		// tris[0] should be triangle on left side.
		Triangle<cplx>* tris[2] = {};
		int direction; // See enum Direction for meaning of values. 
		int dq = 0; // Difference in quadrant of value for this edge's nodes.
	};

	// Triangles for finding boundary regions. Not used for constructing or
	// refining the mesh. These are only created after those things are done.
	template <typename cplx>
	class Triangle
	{
	public:
		Triangle(Edge<cplx>* a, Side a_side, Edge<cplx>* b,
			Side b_side, Edge<cplx>* c, Side c_side);
		Triangle<cplx>* get_adjacent(int index) const;
		Edge<cplx>* get_edge(int index) const;
		// is any edge external
		bool is_external();
		// is the edge at this index external
		bool is_external(int index);
		void make_CCW();

		cplx get_center();

		bool visited = false;
	private:
		Edge<cplx>* edges[3];
	};
};
namespace std
{
	template <typename data_t>
	class hash<std::complex<data_t>> {
	public:
		size_t operator()(const std::complex<data_t>& k) const
		{
			return hash<data_t>()(real(k)) ^ hash<data_t>()(imag(k));
		}
	};
};
namespace zf
{
	template <typename cplx>
	class Mesh
	{
	public:
		typedef typename get_param<cplx>::type data_t;

		Mesh(cplx corner1, cplx corner2,
			typename get_param<cplx>::type init_prec,
			typename get_param<cplx>::type final_prec,
			std::function<cplx(cplx)> func,
			int edge_limit = 0);

		Node<cplx>* insert_node(cplx location);

		// Creates a get_node at the midpoint of this edge. Chosen edge will run
		// from n1 to the new point, a new edge will be created from n2 to the
		// new point.
		Node<cplx>* split(Edge<cplx>* edge/*,
			typename get_param<cplx>::type proportion = 0.5*/);

		// Creates an edge between two nodes, with direction specified from n1
		// to n2.
		void connect(Node<cplx>* n1, Node<cplx>* n2, int dir);

		// Looks for edges that would form triangles to the left and right of 
		// this edge. Creates them if they do not exist. Creates new nodes in 
		// temp_nodes if necessary, and searches there first to avoid
		// duplicating new nodes.
		void complete_quad(Edge<cplx>* e);
		// Calls complete_quad<Edge<T>*) with a temporary edge between the two
		// given nodes.
		void complete_quad(Node<cplx>* n0, Node<cplx>* n1, int dir);

		// Returns estimated zeros/poles. pair includes the location of the
		// zero/pole and the order (negative for poles, zero for a regular point
		// or a zero/pole pair).
		std::vector<std::pair<cplx, int>> find_zeros_and_poles();

		// Increases the resolution of the mesh around the candidate edges to
		// improve the accuracy of the estimation. Call repeatedly as necessary.
		void adapt_mesh();

		// Brings candidate edges to the front and returns an iterator to the
		// end of candidate edges.
		auto candidates_to_front();

		// Creates triangles bordering each candidate edge. Requires those edges
		// to exist. Reorders edges to put the candidates at the front as a
		// side effect.
		auto create_region_triangles();

		// Does not preserve order of edges. If candidate edges have incomplete
		// triangles on either side, complete them recursively (i.e 
		// complete_triangles again for newly any generated candidates). Returns
		// an iterator to the end of the candidate edges.
		void extend_mesh();

		void clear_flags();

		// Removes all edges that are not marked as split (and therefore not
		// in a candidate region). Some edges near but outside the region will
		// remain.
		void cull_edges();

		cplx gen_key(cplx z);

		size_t get_edge_count() { return edges.size(); }

		// Gets pi to the precision of the data type, assuming atan() is
		// accurate.
		inline static const data_t PI = 4 * atan(data_t(1.0));
		inline static const data_t PI_2 = 2 * atan(data_t(1.0));
	private:
		std::unordered_map<cplx, std::unique_ptr<Node<cplx>>> nodes;
		std::vector<std::unique_ptr<Edge<cplx>>> edges;
		std::vector<std::unique_ptr<Triangle<cplx>>> triangles;

		std::function<cplx(cplx)> f;
		data_t precision;
		int edge_limit;
	};

	template<typename cplx>
	inline Mesh<cplx>::Mesh(cplx corner1, cplx corner2,
		typename get_param<cplx>::type edge_width,
		typename get_param<cplx>::type final_prec,
		std::function<cplx(cplx)> func, int edge_lim)
		: precision(final_prec), f(func), edge_limit(edge_lim)
	{
		typedef typename get_param<cplx>::type T;
		// ensures corner1 is the top left and corner2 is bottom right.
		if (real(corner2) < real(corner1))
		{
			T temp_real = real(corner1);
			corner1 = cplx(real(corner2), imag(corner1));
			corner2 = cplx(temp_real, imag(corner2));
		}
		if (imag(corner2) > imag(corner1))
		{
			T temp_imag = imag(corner1);
			corner1 = cplx(real(corner1), imag(corner2));
			corner2 = cplx(real(corner2), temp_imag);
		}

		const T width = real(corner2) - real(corner1);
		const T height = imag(corner1) - imag(corner2);

		const int col_count = (int)ceil(width / edge_width);

		// The mesh will be constructed as an equilateral triangular grid, so we
		// need the height of the triangles.
		const T row_width = edge_width * sin(PI / data_t(3.0));

		const int row_count = 1 + (int)ceil(height / row_width);
		const int node_count = row_count * col_count;
		// 100 is an arbitrary bit of extra space for new points in
		// adapted mesh.
		nodes.reserve(node_count + 100);
		// Likewise, 200 is arbitrary.
		edges.reserve(3 * node_count - 2 * (row_count + col_count) + 1 + 200);

		const T half_edge_width = edge_width / 2;
		T loc_y = imag(corner1);
		T loc_x;

		// Lambdas for readability in the following get_node connection section.

		auto connect_R = [=](T loc_x, T loc_y)
		{
			connect(insert_node(cplx(loc_x, loc_y)),
				insert_node(cplx(loc_x + edge_width, loc_y)),
				(int)Direction::right);
		};

		auto connect_DL = [=](T loc_x, T loc_y)
		{
			connect(insert_node(cplx(loc_x, loc_y)),
				insert_node(cplx(loc_x - half_edge_width,
					loc_y - row_width)),
					(int)Direction::down_left);
		};

		auto connect_DR = [=](T loc_x, T loc_y)
		{
			connect(insert_node(cplx(loc_x, loc_y)),
				insert_node(cplx(loc_x + half_edge_width,
					loc_y - row_width)),
					(int)Direction::down_right);
		};

		// Nodes are connected in an equilateral triangular grid, with the bases
		// aligned with the horizontal axis.

		//  <----------->   Repeat as necessary
		//   ----- ----- $ 
		//  / \   / \   $   <--- Even rows
		// /   \ /   \ $
		// ~~~~~ ----- #
		//  ~   / \   # #   <--- Odd rows
		//   ~ /   \ #   #
		//     ***** *****  <--- Last row
		//      <->   Repeat as necessary
		int x, y;
		loc_y = imag(corner1);
		for (y = 0; y < row_count - 1; y++)
		{
			x = 0;
			if (y % 2) loc_x = real(corner1) - half_edge_width;
			else loc_x = real(corner1);
			if (y % 2)
			{
				//  ~~~~~
				//   ~
				//    ~ 
				connect_R(loc_x, loc_y);
				connect_DR(loc_x, loc_y);
				x++;
				loc_x += edge_width;
			}
			for (x; x < col_count - 1; x++)
			{
				//   -----
				//  / \
				// /   \ 
				connect_R(loc_x, loc_y);
				connect_DL(loc_x, loc_y);
				connect_DR(loc_x, loc_y);
				loc_x += edge_width;
			}
			if (y % 2)
			{
				//   #
				//  # #
				// #   #
				connect_DR(loc_x, loc_y);
				connect_DL(loc_x, loc_y);
			}
			else
				//   $
				//  $
				// $
				connect_DL(loc_x, loc_y);
			loc_y -= row_width;
		}
		if (y % 2) loc_x = real(corner1) - half_edge_width;
		else loc_x = real(corner1);
		for (x = 0; x < col_count - 1; x++)
		{
			//
			// *****
			//
			connect_R(loc_x, loc_y);
			loc_x += edge_width;
		}

		extend_mesh();
	}

	template<typename cplx>
	inline Node<cplx>* Mesh<cplx>::insert_node(cplx location)
	{
		return nodes.insert({
			gen_key(location),
			std::make_unique<Node<cplx>>(
			location, f(location)) }).first->second.get();
	}

	template<typename cplx>
	inline Node<cplx>* Mesh<cplx>::split(Edge<cplx>* edge)
	{
		 auto N = insert_node((edge->get_node(0)->location +
			 edge->get_node(1)->location) / data_t(2.0));
		connect(edge->get_node(1), N, edge->get_dir() + 3);
		edge->set_node(1, N);
		edge->is_new = true;
		edges.back()->is_new = true;
		return N;
	}

	template<typename cplx>
	inline void Mesh<cplx>::connect(Node<cplx>* n1, Node<cplx>* n2, int dir)
	{
		edges.push_back(std::make_unique<Edge<cplx>>(n1, n2, mod(dir, 6)));
		n1->set_edge(edges.back().get(), dir);
		n2->set_edge(edges.back().get(), dir + 3);
	}

	template<typename cplx>
	inline void Mesh<cplx>::complete_quad(Edge<cplx>* e)
	{
		auto add_edges = [&](Side side) {
			int n0 = 0, n1 = 1;
			int dir_change1 = 1, dir_change2 = 2;
			if (side == Side::left)
			{
				std::swap(n0, n1);
				std::swap(dir_change1, dir_change2);
			}
			auto neg = n0 - n1;

			// Can't have a candidate edge on the edge of the mesh, so we
			// apply the same process to extend the newest edge if it is one.
			auto recurse_if_candidate = [&](Edge<cplx>* e)
			{
				if (abs(e->get_dq()) == 2)
				{
					complete_quad(e);
				}
			};

			// If one edge does not exist, uses the second point from the other
			// edge to create it.
			auto add_one_edge = [&](Side CW) {
				int dir = (bool)CW ? -1 : 1;
				int dir_change = (bool)CW ? dir_change2 : dir_change1;
				int n_0 = n0;
				int n_1 = n1;
				if (CW == Side::right) std::swap(n_0, n_1);

				// N needs to be the node not shared with e, so we store one,
				// check it, and if matches, store the other, instead.
				auto e2 = e->get_next(n_1, dir);
				auto N = e2->get_node(n_1);
				if (N == e->get_node(n_1))
					N = e2->get_node(n_0);
				//auto elen = e->length();
				//auto e2len = e2->length();
				if (!e2->is_new)
				{
					connect(e->get_node(n_0), N, e->get_dir()
						+ neg * dir_change);
					recurse_if_candidate(edges.back().get());
				}
				else
				{
					e2 = e2->get_continuation();
					connect(e->get_node(n_0), e2->get_node(0),
						e->get_dir() + neg * dir_change);
					recurse_if_candidate(edges.back().get());
				}
			};

			// If neither edge of the triangle on the target side exists, then
			// add a node (or find one in temp_nodes) and create the edges.
			if (!e->get_next_CCW(n1) && !e->get_next_CW(n0))
			{
				// places the node by taking e's node 0's location and
				// adding a number with the same length as e, rotated
				// appropriately.
				auto N = insert_node(e->get_node(n0)->location + e->length()
					* exp(data_t((e->get_dir() + dir_change1 * neg))
						 * cplx(0, PI) / data_t(3.0)));

				connect(e->get_node(n0), N, e->get_dir() + neg * dir_change1);
				recurse_if_candidate(edges.back().get());
				connect(e->get_node(n1), N, e->get_dir() + neg * dir_change2);
				recurse_if_candidate(edges.back().get());
			}
			else if (!e->get_next_CW(n0))
			{
				add_one_edge(Side::left);
			}
			else if (!e->get_next_CCW(n1))
			{
				add_one_edge(Side::right);
			}
		};

		add_edges(Side::right);
		add_edges(Side::left);
	}

	template<typename cplx>
	inline void Mesh<cplx>::
		complete_quad(Node<cplx>* n0, Node<cplx>* n1, int dir)
	{
		Edge<cplx> e(n0, n1, dir);
		complete_quad(&e);
		e.detach_nodes();
	}

	template<typename cplx>
	inline std::vector<std::pair<cplx, int>> Mesh<cplx>::find_zeros_and_poles()
	{
		auto candidates_end = create_region_triangles();

		// Finds the order for each candidate region by looking at a constituent
		// triangle, adding up the dqs for each external edge, then recursively
		// checking each adjacent triangle on the internal edges. The function
		// estimates the location of a zero/pole by averaging the boundary
		// points and the order by adding up the dq values for each edge
		// (4 quadrants = 1 full turn).

		std::vector<std::pair<cplx, int>> points;

		std::for_each(triangles.begin(), triangles.end(), [&](auto&& triangle)
			{
				// Center of triangle is just a convenient point 
				// guaranteed to be inside its region.
				cplx interior_point = triangle->get_center();
				int boundarysize = 0;
				float sum_dq = 0;
				cplx node_sum = cplx(0.0);
				Edge<cplx>* next;
				std::function<void(Triangle<cplx>*)> next_tri =
					[&](Triangle<cplx>* tri)
				{
					if (tri && !tri->visited)
					{
						tri->visited = true;
						tri->make_CCW();
						for (int i = 0; i < 3; i++)
						{
							if (tri->is_external(i))
							{
								boundarysize++;
								next = tri->get_edge(i);
								node_sum += next->get_node(0)->location
									+ next->get_node(1)->location;
								sum_dq += next->get_dq();
								for (int j = 2; j < 6; j++)
								{
									auto touching = next->get_node(0)
										->get_edge(next->get_dir() + j);
									if (touching)
									{
										next_tri(touching->get_tri(0));
										next_tri(touching->get_tri(1));
									}
									touching = next->get_node(1)
										->get_edge(next->get_dir() + j);
									if (touching)
									{
										next_tri(touching->get_tri(0));
										next_tri(touching->get_tri(1));
									}
								}
							}
							else
								next_tri(tri->get_adjacent(i));
						}
					}
				};
				next_tri(triangle.get());

				if (boundarysize)
				{
					points.push_back(std::make_pair(node_sum
						/ (cplx)(boundarysize * 2), sum_dq / 4));
				}
			});

		return points;
	}

	template<typename cplx>
	inline void Mesh<cplx>::adapt_mesh()
	{
		auto candidate_end = candidates_to_front();

		auto dist = std::distance(edges.begin(), candidate_end);

		// Splits the candidate edges in half.
		for (int i = 0; i < dist; i++)
		{
			split(edges[i].get());
		}

		// Splits all the visited edges in half, and connect the midpoints.
		//
		//    /\
		//   /__\
		//  /\  /\
		// /__\/__\
		// \  /\  /
		//  \/__\/
		//   \  /
		//    \/
		int recursion_depth;
		std::function<void(Edge<cplx>*)> subdivide = [&](Edge<cplx>* e) {
			// Under normal circumstances, recursion_depth shouldn't exceed 2-3.
			// However, FP instability seems to make it go nuts under some
			// circumstances, hence the hard cutoff. Chances of this messing up
			// a legitimate boundary are low but theoretically nonzero.
			if (recursion_depth > 6) return;

			auto e2 = e->get_continuation();
			auto split_and_get_end_node = [&](Edge<cplx>* E)
			{
				if (!E->is_new)
				{
					split(E);
				}
				return E->get_node(1);
			};
			
			complete_quad(e->get_node(0), e2->get_node(0), e->get_dir());

			int dir = e->get_dir();

			std::pair<Node<cplx>*, int> new_nodes[4] = {
			{ split_and_get_end_node(e->get_next_CW(0)), (dir - 2) },
			{ split_and_get_end_node(e2->get_next_CCW(0)), (dir - 1) },
			{ split_and_get_end_node(e2->get_next_CW(0)), (dir + 1) },
			{ split_and_get_end_node(e->get_next_CCW(0)), (dir + 2) },
			};

			Edge<cplx>* new_edges[8] = {
				e->get_next_CW(0), e->get_next_CW(0)->get_continuation(),
				e2->get_next_CCW(0), e2->get_next_CCW(0)->get_continuation(),
				e2->get_next_CW(0), e2->get_next_CW(0)->get_continuation(),
				e->get_next_CCW(0), e->get_next_CCW(0)->get_continuation()
			};

			auto candidate_midpoint = e->get_node(1);
			//     .
			//    . .
			//   \   /
			//  . \ / .
			// ....X....
			//  . / \ .
			//   /   \
			//    . .
			//     .
			for (auto& N : new_nodes)
			{
				if (!candidate_midpoint->get_edge(N.second))
				{
					connect(candidate_midpoint, N.first, N.second);
					edges.back()->is_new = true;
				}
			}
			//     .
			//    . .
			//   .___.
			//  . . . .
			// .........
			//  . . . .
			//   .___.
			//    . .
			//     .
			if (!new_nodes[0].first->get_edge(dir))
			{
				connect(new_nodes[0].first, new_nodes[1].first, dir);
				edges.back()->is_new = true;
			}
			if (!new_nodes[3].first->get_edge(dir))
			{
				connect(new_nodes[3].first, new_nodes[2].first, dir);
				edges.back()->is_new = true;
			}
			for (auto ne : new_edges)
			{
				if (!ne->boundary && abs(ne->get_dq()) == 2)
				{
					recursion_depth++;
					ne->boundary = true;
					subdivide(ne);
				}
			}
		};

		for (int i = 0; i < dist; i++)
		{
			recursion_depth = 0;
			subdivide(edges[i].get());
			if (edge_limit > 0 && get_edge_count() > edge_limit)
				throw std::exception("Edge count exceeded! Specify a higher "
					"value (uses more memory), reduce "
					"precision, or set a smaller region.");
		}
	}

	template<typename cplx>
	inline auto Mesh<cplx>::candidates_to_front()
	{
		return std::partition(edges.begin(), edges.end(),
			[](auto&& e)
			{
				return abs(e->get_dq()) == 2;
			});
	}

	template<typename cplx>
	inline auto Mesh<cplx>::create_region_triangles()
	{
		auto candidate_end = candidates_to_front();

		// For each candidate edge, create the triangles bordering it, if
		// it doesn't already exist.

		std::for_each(edges.begin(), candidate_end, [&](auto&& e)
			{
				e->boundary = true;
				auto e1 = e->get_next_CW(0);
				Side e1_side;
				if (e1->get_node(0) == e->get_node(0))
					e1_side = Side::left;
				else
					e1_side = Side::right;

				Side e2_side;
				auto e2 = e->get_next_CCW(1);
				if (e2->get_node(1) == e->get_node(1))
					e2_side = Side::left;
				else
					e2_side = Side::right;

				Side e3_side;
				auto e3 = e->get_next_CW(1);
				if (e3->get_node(1) == e->get_node(1))
					e3_side = Side::right;
				else
					e3_side = Side::left;

				Side e4_side;
				auto e4 = e->get_next_CCW(0);
				if (e4->get_node(0) == e->get_node(0))
					e4_side = Side::right;
				else
					e4_side = Side::left;
				if (!e->get_tri(1))
					triangles.push_back(std::make_unique<Triangle<cplx>>(
						e.get(), Side::right, e1, e1_side, e2, e2_side));
				if (!e->get_tri(0))
					triangles.push_back(std::make_unique<Triangle<cplx>>(
						e.get(), Side::left, e3, e3_side, e4, e4_side));
			});

		return candidate_end;
	}

	template<typename cplx>
	inline void Mesh<cplx>::extend_mesh()
	{
		auto C = candidates_to_front();
		auto dist = std::distance(edges.begin(), C);
		for (int i = 0; i < dist; i++)
		{
			auto e = edges[i].get();
			if (!(e->get_next_CW(0) && e->get_next_CCW(1) &&
				e->get_next_CCW(0) && e->get_next_CW(1))) complete_quad(e);
		};
	}

	template<typename cplx>
	inline void Mesh<cplx>::clear_flags()
	{
		std::for_each(/*std::execution::par_unseq,*/ edges.begin(), edges.end(),
			[](auto&& e)
			{
				e->boundary = false;
				e->is_new = false;
			});
	}

	template<typename cplx>
	inline void Mesh<cplx>::cull_edges()
	{
		edges.erase(std::remove_if(edges.begin(), edges.end(), [](auto&& e)
			{
				return !e->is_new;
			}), edges.end());

		// Remove unused nodes, i.e. ones with no edges connected.
		// Keeps memory usage down somewhat.
		auto itr = nodes.begin();
		auto end = nodes.end();
		while (itr != end)
		{
			auto n = (*itr).second.get();
			if (!n->get_edge(0) && !n->get_edge(1)
				&& !n->get_edge(2) && !n->get_edge(3)
				&& !n->get_edge(4) && !n->get_edge(5))
				itr = nodes.erase(itr);
			else itr++;
		}
		nodes.rehash(0);
	}

	template<typename cplx>
	inline cplx Mesh<cplx>::gen_key(cplx z)
	{
		data_t a = trunc(real(z) / precision * data_t(4.0) + data_t(0.5));
		data_t b = trunc(imag(z) / precision * data_t(4.0) + data_t(0.5));
		return cplx(a, b);
	}

	template<typename cplx>
	inline Edge<cplx>::Edge(Node<cplx>* node1, Node<cplx>* node2, int dir)
		: nodes{ node1, node2 }, direction(dir)
	{
		calc_dq();
	}

	template<typename cplx>
	inline Edge<cplx>::~Edge()
	{
		if (nodes[0]) nodes[0]->set_edge(nullptr, direction);
		if (nodes[1]) nodes[1]->set_edge(nullptr, direction + 3);
	}

	template<typename cplx>
	inline Node<cplx>* Edge<cplx>::get_node(int index) const
	{
		return nodes[index];
	}

	template<typename cplx>
	inline void Edge<cplx>::set_node(int i, Node<cplx>* N)
	{
		nodes[i] = N;
		N->set_edge(this, direction + i * 3);
		calc_dq();
	}

	template<typename cplx>
	inline Triangle<cplx>* Edge<cplx>::get_tri(int index) const
	{
		return tris[index];
	}

	template<typename cplx>
	inline void Edge<cplx>::set_tri(Side index, Triangle<cplx>* tri)
	{
		tris[(int)index] = tri;
	}

	template<typename cplx>
	inline int Edge<cplx>::get_dir()
	{
		return direction;
	}

	template<typename cplx>
	inline Edge<cplx>* Edge<cplx>::get_next_CW(int n) const
	{
		return this->get_node(n)->get_edge(this->direction - 1 + 3 * n);
	}

	template<typename cplx>
	inline Edge<cplx>* Edge<cplx>::get_next_CCW(int n) const
	{
		return this->get_node(n)->get_edge(this->direction + 1 + 3 * n);
	}

	template<typename cplx>
	inline Edge<cplx>* Edge<cplx>::get_continuation()
	{
		return nodes[1]->get_edge(direction);
	}

	template<typename cplx>
	inline Edge<cplx>* Edge<cplx>::get_next(int n, int steps)
	{
		return this->get_node(n)->get_edge(this->direction + steps + 3 * n);
	}

	template<typename cplx>
	inline void Edge<cplx>::reverse()
	{
		std::swap(nodes[0], nodes[1]);
		std::swap(tris[0], tris[1]);
		direction += 3;
		direction %= 6;
		dq *= -1;
	}

	template<typename cplx>
	inline auto Edge<cplx>::length()
	{
		return abs(nodes[1]->location - nodes[0]->location);
	}

	template<typename cplx>
	inline void Edge<cplx>::detach_nodes()
	{
		nodes[0] = nullptr;
		nodes[1] = nullptr;
	}

	template<typename cplx>
	inline int Edge<cplx>::get_dq() const
	{
		return dq;
	}

	template<typename cplx>
	inline void Edge<cplx>::calc_dq()
	{
		dq = (nodes[1]->q - nodes[0]->q);
		if (dq > 2) dq -= 4;
		else if (dq < -2) dq += 4;
	}

	template<typename cplx>
	inline std::vector<std::pair<cplx, int>>
		solve(cplx ULcorner, cplx LRcorner,
			typename get_param<cplx>::type precision,
			std::function<cplx(cplx)> f,
			typename get_param<cplx>::type initial_mesh_len,
			int edge_limit)
	{
		typedef typename get_param<cplx>::type data_t;

		//auto range = std::max(abs(real(ULcorner) - real(LRcorner)),
		//	abs(imag(ULcorner) - imag(LRcorner)));

		// If the specified precision is too fine to actually be usable with
		// the given data type over the specified range, grow it until it works.
		// This is very crude but at least it does not depend on any details of
		// the underlying data type.

		data_t limits[4] = {real(ULcorner), imag(ULcorner),
							real(LRcorner), imag(LRcorner)};
		bool prec_to_low;
		bool increased_prec = false;
		do
		{
			prec_to_low = false;
			for (auto lim : limits)
			{
				auto lim2 = lim + precision / data_t(4.0);
				auto test1 = trunc(lim2 / precision * data_t(4.0) + 0.5);
				auto test2 = trunc(lim / precision * data_t(4.0) + 0.5);
				if (test1 == test2)
				{
					prec_to_low = true;
					increased_prec = true;
				}
			}
			if (prec_to_low)
				precision *= 2;
		} while (prec_to_low == true);

		// Trying to make some room for FP instability.
		if (increased_prec) precision *= 1024;

		//// At high precisions relative to the data type, floating point
		//// instability becomes an issue and can cause the number of edges
		//// to explode. When or if this happens depends on the specific function
		//// being analyzed. Past the initial refinement of the mesh, if the
		//// number of edges exceeds edge_limit, the refinement step terminates
		//// early.
		//const int edge_limit = 2000;

		// Arbitrary. Later could add a trial-and error mesh sizer which tests
		// a few values in the desired neighborhood and picks the one which
		// finds the most candidate regions.
		if (initial_mesh_len < data_t(0.0))
			initial_mesh_len = std::min(abs(real(ULcorner) - real(LRcorner))
				/ data_t(40.0), abs(imag(ULcorner)
					- imag(LRcorner)) / data_t(40.0));
		int iterations = (int)ceil(log(data_t(log(2.0))
			* initial_mesh_len / precision));

		Mesh<cplx> mesh(ULcorner, LRcorner, initial_mesh_len, precision, f, edge_limit);

		for (int i = 0; i < iterations; i++)
		{
			mesh.adapt_mesh();
			mesh.cull_edges();
			mesh.clear_flags();
			//if (i > 2 && mesh.get_edge_count() > edge_limit)
			//	break;
		}
		return mesh.find_zeros_and_poles();
	}
	template<typename cplx>
	inline Triangle<cplx>::Triangle(Edge<cplx>* a, Side a_side, Edge<cplx>* b,
		Side b_side, Edge<cplx>* c, Side c_side)
		: edges{ a, b, c }
	{
		a->set_tri(a_side, this);
		b->set_tri(b_side, this);
		c->set_tri(c_side, this);
	}
	template<typename cplx>
	inline Triangle<cplx>* Triangle<cplx>::get_adjacent(int index) const
	{
		auto edge = edges[index];
		if (this == edge->get_tri(0))
			return edge->get_tri(1);
		else return edge->get_tri(0);
	}
	template<typename cplx>
	inline Edge<cplx>* Triangle<cplx>::get_edge(int index) const
	{
		return edges[index];
	}
	template<typename cplx>
	inline bool Triangle<cplx>::is_external()
	{
		return get_adjacent(0) || get_adjacent(1) || get_adjacent(2);
	}
	template<typename cplx>
	inline bool Triangle<cplx>::is_external(int index)
	{
		return !get_adjacent(index);
	}
	template<typename cplx>
	inline void Triangle<cplx>::make_CCW()
	{
		for (auto e : edges)
		{
			if (e->get_tri(0) != this) e->reverse();
		}
	}
	template<typename cplx>
	inline cplx Triangle<cplx>::get_center()
	{
		typedef typename get_param<cplx>::type data_t;
		cplx res = cplx(0.0);
		for (auto e : edges)
			res += e->get_node(0)->location;
		return res / data_t(3.0);
	}
	template<typename cplx>
	inline Node<cplx>::Node(cplx loc, cplx val)
		: location(loc)
	{
		calc_q(val);
	}
	template<typename cplx>
	inline Edge<cplx>* Node<cplx>::get_edge(int dir)
	{
		return edges[mod(dir, 6)];
	}
	template<typename cplx>
	inline void Node<cplx>::set_edge(Edge<cplx>* e, int dir)
	{
		edges[mod(dir, 6)] = e;
	}
	template<typename cplx>
	inline void Node<cplx>::calc_q(cplx z)
	{
		typedef typename get_param<cplx>::type data_t;
		if (data_t a = arg(z); a >= 0)
			q = (int)ceil(a / Mesh<cplx>::PI_2);
		else q = 4 + (int)ceil(a / Mesh<cplx>::PI_2);
	}
}