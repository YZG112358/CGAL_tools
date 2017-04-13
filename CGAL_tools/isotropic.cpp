#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <boost/function_output_iterator.hpp>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

#define  X(v)  (mesh.point(v)).x()
#define  Y(v)  (mesh.point(v)).y()
#define  Z(v)  (mesh.point(v)).z()
#define  epsion 0.01


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<Mesh>::edge_descriptor     edge_descriptor;
typedef boost::graph_traits<Mesh>::vertex_descriptor     vertex_descriptor;
namespace PMP = CGAL::Polygon_mesh_processing;
using namespace std;
struct halfedge2edge
{
  halfedge2edge(const Mesh& m, std::vector<edge_descriptor>& edges)
    : m_mesh(m), m_edges(edges)
  {}
  void operator()(const halfedge_descriptor& h) const
  {
    m_edges.push_back(edge(h, m_mesh));
  }
  const Mesh& m_mesh;
  std::vector<edge_descriptor>& m_edges;
};

bool segmentOnEdge(edge_descriptor& target, std::vector<edge_descriptor>& pe, Mesh& mesh){
    auto v0 = mesh.vertex(target, 0); 
    auto v1 = mesh.vertex(target, 1);
    auto it = pe.begin();
    for (; it != pe.end(); it++){
        auto ev0 = mesh.vertex(*it, 0);
        auto ev1 = mesh.vertex(*it, 1);
        // to avoid divide 0
        int cnt0 = (X(v0) == X(ev0)) + (Y(v0) == Y(ev0)) + (Z(v0) == Z(ev0));
        int cnt1 = (X(v1) == X(ev1)) + (Y(v1) == Y(ev1)) + (Z(v1) == Z(ev1));
        if (cnt0 == 2 && cnt1 == 2) return true;

        double pex = abs(X(ev0) - X(ev1));
        double pey = abs(Y(ev0) - Y(ev1));
        double pez = abs(Z(ev0) - Z(ev1));
        
        double ex = abs(X(v0) - X(v1));
        double ey = abs(Y(v0) - Y(v1));
        double ez = abs(Z(v0) - Z(v1));
        
        double res1 = ((ex != 0) ? pex / ex : 0);
        double res2 = ((ey != 0) ? pey / ey : 0);
        double k1 = (Y(v1) - Y(v0)) / (X(v1) - X(v0));
        double k2 = (Y(ev1) - Y(ev0)) / (X(ev1) - X(ev0));
        if (abs(k1 - k2) <= epsion){
            double k = (Y(ev0) - Y(ev1)) / (X(ev0) - X(ev1));
            // pick the point with smaller x
            double orix = X(ev0) < X(ev1) ? X(ev0) : X(ev1);
            double oriy = X(ev0) < X(ev1) ? Y(ev0) : Y(ev1);
            double expectedx = X(v0) < X(v1) ? X(v0) : X(v1);
            double expectedy = oriy + k * (expectedx - orix);
            double realy = X(v0) < X(v1) ? Y(v0) : Y(v1);
            if (abs(expectedy - realy) < epsion) return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
  Mesh mesh;
/*
  vertex_descriptor vd[12];
  vd[0] = mesh.add_vertex(K::Point_3(0,0,0));   // cons
  vd[1] = mesh.add_vertex(K::Point_3(1,0,0));
  vd[2] = mesh.add_vertex(K::Point_3(2,0,0));
  vd[3] = mesh.add_vertex(K::Point_3(0,1,0));
  vd[4] = mesh.add_vertex(K::Point_3(1,1,0));   // cons
  vd[5] = mesh.add_vertex(K::Point_3(2,1,0));
  vd[6] = mesh.add_vertex(K::Point_3(0,2,0));
  vd[7] = mesh.add_vertex(K::Point_3(1,2,0));   // cons
  vd[8] = mesh.add_vertex(K::Point_3(2,2,0));
  vd[9] = mesh.add_vertex(K::Point_3(0,3,0));
  vd[10] = mesh.add_vertex(K::Point_3(1,3,0));
  vd[11] = mesh.add_vertex(K::Point_3(2,3,0));  // cons

  mesh.add_face(vd[0], vd[1], vd[4]);
  mesh.add_face(vd[1], vd[2], vd[5]);
  mesh.add_face(vd[0], vd[4], vd[3]);
  mesh.add_face(vd[1], vd[5], vd[4]);
  mesh.add_face(vd[3], vd[4], vd[7]);
  mesh.add_face(vd[4], vd[5], vd[8]);
  mesh.add_face(vd[3], vd[7], vd[6]);
  mesh.add_face(vd[4], vd[8], vd[7]);
  mesh.add_face(vd[6], vd[7], vd[10]);
  mesh.add_face(vd[7], vd[8], vd[11]);
  mesh.add_face(vd[6], vd[10], vd[9]);
  mesh.add_face(vd[7], vd[11], vd[10]);
*/

  const char* filename = (argc > 1) ? argv[1] : "data/OFF2.off";
  std::ifstream input(filename);
  if (!input || !(input >> mesh)) {
    std::cerr << "Not a valid off file." << std::endl;
    return 1;
  }

  double target_edge_length;
  std::cout << "input tartget_edge_length: ";
  std::cin >> target_edge_length;
  unsigned int nb_iter = 3;
  //std::cin >> nb_iter;
  std::cout << "Split border...";

  std::vector<edge_descriptor> border;
  std::vector<edge_descriptor> protected_edges;


  std::map<edge_descriptor, bool> edgeMap;
  boost::associative_property_map< std::map<edge_descriptor, bool> >
    edge_map(edgeMap);

  std::map<vertex_descriptor, bool> vertexMap;
  boost::associative_property_map< std::map<vertex_descriptor, bool> >
    vertex_map(vertexMap);


  //std::list<edge_descriptor> mst;
  //boost::kruskal_minimum_spanning_tree(mesh, 
  //                                     std::back_inserter(mst));
  //fprintf(stderr,"Spanning finished\n");
  //Mesh::Property_map<edge_descriptor,bool> edge_map;

  Mesh::Vertex_range::iterator vb, ve;
  Mesh::Vertex_range rv = mesh.vertices();
  vb = rv.begin();

auto itt = vb;
/*
    for (; itt != rv.end(); itt++){
        vertexMap.insert(std::make_pair(*itt, true));
    }
*/
  Mesh::Edge_range::iterator  eb, ee;
  Mesh::Edge_range r = mesh.edges();
  eb = r.begin();
  ee = r.end();
  auto it = eb;

// Let's split the outiside boundary

double outside_boundary_split_length = 0.1;
  std::cin >> outside_boundary_split_length;
/*
PMP::border_halfedges(faces(mesh),
      mesh,
      boost::make_function_output_iterator(halfedge2edge(mesh, border)));
*/
// split the selected border
  for (; it != r.end(); it++){
        border.push_back(*it);
        protected_edges.push_back(*it);
  }
printf("boder size = %d\n", border.size());
  
PMP::split_long_edges(border, outside_boundary_split_length, mesh);

// Get all the edges into the map  
  r = mesh.edges();
  it = r.begin();

int cnt = 0;

  for (; it != r.end(); it++){
    if (segmentOnEdge(*it, protected_edges, mesh)){
      edgeMap.insert(std::make_pair(*it, 
          true));
        cnt ++;
    }
  }
printf("num of edges %d\n", cnt);


// Set my constrained edge ends
/*  
  edgeMap.insert(std::make_pair(mesh.edge(mesh.halfedge(vd[0], vd[4])), 
          true));
  edgeMap.insert(std::make_pair(mesh.edge(mesh.halfedge(vd[7], vd[11])), 
          true));
*/


/*
  for(std::list<edge_descriptor>::iterator it = mst.begin(); it != mst.end(); ++it)
  {
    edge_descriptor e = *it ;
    edgeMap.insert(std::make_pair(e, 
        true));
    border.push_back(*it);
  }
*/
 
  std::cout << "done." << std::endl;
  std::cout << "Start remeshing of "
    << " (" << num_faces(mesh) << " faces)..." << std::endl;
 
  PMP::isotropic_remeshing(
      faces(mesh),
      target_edge_length,
      mesh,
      PMP::parameters::number_of_iterations(nb_iter)
      .vertex_is_constrained_map(vertex_map)
      .edge_is_constrained_map(edge_map)
      .protect_constraints(true)//i.e. protect border, here
      );

  //PMP::split_long_edges(border, target_edge_length, mesh);
  
  std::ofstream iso_off("isotropic.off");
  iso_off << mesh;
  iso_off.close();
  std::cout << "Remeshing done." << std::endl;
  return 0;
}
