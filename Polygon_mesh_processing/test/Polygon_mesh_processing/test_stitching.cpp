// #define CGAL_PMP_STITCHING_DEBUG_PP

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/boost/graph/named_params_helper.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/Dynamic_property_map.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>

#include <iostream>
#include <iterator>
#include <fstream>
#include <unordered_map>

namespace PMP = CGAL::Polygon_mesh_processing;
namespace params = CGAL::parameters;

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Mesh>
void test_stitch_boundary_cycles(const char* fname,
                                 const std::size_t expected_n)
{
  typedef typename boost::graph_traits<Mesh>::vertex_descriptor       vertex_descriptor;
  typedef typename boost::graph_traits<Mesh>::halfedge_descriptor     halfedge_descriptor;
  typedef typename boost::graph_traits<Mesh>::face_descriptor         face_descriptor;

  std::cout << "Testing stitch_boundary_cycles(); file: " << fname << "..." << std::flush;

  std::ifstream input(fname);
  Mesh mesh;
  if (!input || !(input >> mesh)){
    std::cerr << "Error: can not read file.";
    return;
  }

  // stitching a single cycle should work from any border halfedge
  for(halfedge_descriptor h : halfedges(mesh))
  {
    if(!is_border(h, mesh))
      continue;

    std::unordered_map<vertex_descriptor, vertex_descriptor> v2v;
    std::unordered_map<halfedge_descriptor, halfedge_descriptor> h2h;
    std::unordered_map<face_descriptor, face_descriptor> f2f;

    Mesh mesh_cpy;
    CGAL::copy_face_graph(mesh, mesh_cpy,
                          std::inserter(v2v, v2v.end()),
                          std::inserter(h2h, h2h.end()),
                          std::inserter(f2f, f2f.end()));

    assert(is_border(h2h.at(h), mesh_cpy));

    PMP::stitch_boundary_cycle(h2h.at(h), mesh_cpy);
    assert(is_valid_polygon_mesh(mesh_cpy));
  }

  std::size_t res = PMP::stitch_boundary_cycles(mesh);
  std::cout << "res: " << res << " (expected: " << expected_n << ")" << std::endl;

  assert(res == expected_n);
  assert(is_valid_polygon_mesh(mesh));
}

template <typename Mesh>
void test_stitch_boundary_cycles()
{
  test_stitch_boundary_cycles<Mesh>("data_stitching/boundary_cycle.off", 4);
  test_stitch_boundary_cycles<Mesh>("data_stitching/boundary_cycle_2.off", 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Mesh>
void test_stitch_borders(const char* fname,
                         const std::size_t expected_n,
                         const bool per_cc = false,
                         std::set<int> unconstrained_edges = { })
{
  std::cout << "Testing stitch_borders(); file: " << fname << "..." << std::flush;

  std::ifstream input(fname);
  Mesh mesh;
  if (!input || !(input >> mesh))
  {
    std::cerr << "Error: can not read file.";
    return;
  }

  // Mark some edges as UNCONSTRAINED, the stitched mesh must not contain these edges
  typedef typename boost::graph_traits<Mesh>::edge_descriptor         edge_descriptor;
  typedef CGAL::dynamic_edge_property_t<bool>                         Edge_property_tag;
  typedef typename boost::property_map<Mesh, Edge_property_tag>::type Marked_edges;
  typedef PMP::internal::Halfedges_keeper_with_marked_edge_priority<Marked_edges, Mesh> Keeper;

  Marked_edges marks = get(Edge_property_tag(), mesh);
  int id = 0;
  for(edge_descriptor e : edges(mesh))
    put(marks, e, (unconstrained_edges.count(id++) == 0));

  Keeper kpr(marks, mesh);

  std::size_t res = PMP::stitch_borders(mesh, params::apply_per_connected_component(per_cc)
                                                     .halfedges_keeper(kpr));
  std::cout << "res: " << res << " (expected: " << expected_n << ")" << std::endl;

  for(edge_descriptor e : edges(mesh)) {
    assert(get(marks, e)); // must not be marked as unconstrained
  }

  assert(res == expected_n);
  assert(is_valid_polygon_mesh(mesh));
}

template <typename Mesh>
void test_stitch_borders()
{
  test_stitch_borders<Mesh>("data_stitching/deg_border.off", 2, false /*per_cc*/, {9, 12} /*unconstrained edges*/);
  test_stitch_borders<Mesh>("data_stitching/full_border.off", 4);
  test_stitch_borders<Mesh>("data_stitching/full_border_quads.off", 4);
  test_stitch_borders<Mesh>("data_stitching/half_border.off", 2, false, {23, 15});
  test_stitch_borders<Mesh>("data_stitching/incidence_3.off", 3);
  test_stitch_borders<Mesh>("data_stitching/incoherent_patch_orientation.off", 1);
  test_stitch_borders<Mesh>("data_stitching/mid_border.off", 2);
  test_stitch_borders<Mesh>("data_stitching/multiple_incidence.off", 10, false, {3, 5, 16, 24, 31, 45, 53, 65});
  test_stitch_borders<Mesh>("data_stitching/non_stitchable.off", 0);
  test_stitch_borders<Mesh>("data_stitching/non_manifold.off", 0);
  test_stitch_borders<Mesh>("data_stitching/non_manifold2.off", 0);
  test_stitch_borders<Mesh>("data_stitching/two_patches.off", 3);
  test_stitch_borders<Mesh>("data_stitching/nm_cubes.off", 4, true /*per cc*/);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Mesh>
void test_degenerate()
{
  std::cout << "Testing degenerate cases" << std::endl;

  typedef typename boost::property_traits<
    typename CGAL::GetVertexPointMap<Mesh>::const_type>::value_type Point;

  Mesh tm;

  CGAL::make_triangle(Point(0,0,0), Point(1,0,0), Point(0,1,0), tm);
  CGAL::make_triangle(Point(0,0,0), Point(1,0,0), Point(0,1,0), tm);
  CGAL::make_triangle(Point(0,0,0), Point(1,0,0), Point(0,1,0), tm);
  CGAL::make_triangle(Point(0,0,0), Point(1,0,0), Point(0,1,0), tm);

  CGAL::Polygon_mesh_processing::stitch_borders(tm);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Mesh>
void test()
{
  test_stitch_boundary_cycles<Mesh>();
  test_stitch_borders<Mesh>();
  test_degenerate<Mesh>();
}

int main()
{
  typedef CGAL::Exact_predicates_inexact_constructions_kernel     EPICK;

  std::cout << "--- Test EPICK SM" << std::endl;
  test<CGAL::Surface_mesh<EPICK::Point_3> >();

  std::cout << "--- Test EPICK Polyhedron" << std::endl;
  test<CGAL::Polyhedron_3<EPICK> >();

  // Disabled because it takes too long for some test plateforms
//  typedef CGAL::Exact_predicates_exact_constructions_kernel       EPECK;

//  std::cout << "--- Test EPECK SM" << std::endl;
//  test<CGAL::Surface_mesh<EPECK::Point_3> >();

//  std::cout << "--- Test EPECK Polyhedron" << std::endl;
//  test<CGAL::Polyhedron_3<EPECK> >();

  std::cout << "Done!" << std::endl;

  return EXIT_SUCCESS;
}
