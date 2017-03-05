#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Mesh_3/Robust_intersection_traits_3.h>
#include <CGAL/Surface_mesh_default_criteria_3.h>
#include <CGAL/AABB_polyhedral_oracle.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_items_3.h>
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h>
#include <fstream>

template <class HDS>
class Build_triangle : public CGAL::Modifier_base<HDS> {
public:
    Build_triangle() {}
    void operator()( HDS& hds) {
        // Postcondition: hds is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<HDS> B( hds, true);
        B.begin_surface( 4, 4);
        typedef typename HDS::Vertex   Vertex;
        typedef typename Vertex::Point Point;
        B.add_vertex( Point( 0,0,0));
        B.add_vertex( Point( 1,0,0));
        B.add_vertex( Point( 0,1,0));
        B.add_vertex( Point( 0,0,1));

        B.begin_facet();
        B.add_vertex_to_facet( 1);
        B.add_vertex_to_facet( 3);
        B.add_vertex_to_facet( 0);
        B.end_facet();

        B.begin_facet();
        B.add_vertex_to_facet( 1);
        B.add_vertex_to_facet( 2);
        B.add_vertex_to_facet( 3);
        B.end_facet();

        B.begin_facet();
        B.add_vertex_to_facet( 2);
        B.add_vertex_to_facet( 1);
        B.add_vertex_to_facet( 0);
        B.end_facet();

        B.begin_facet();
        B.add_vertex_to_facet( 3);
        B.add_vertex_to_facet( 2);
        B.add_vertex_to_facet( 0);
        B.end_facet();

        B.end_surface();
    }
};

typedef CGAL::Polyhedron_items_3											Customized_items;
typedef CGAL::Exact_predicates_inexact_constructions_kernel     			CGAL_EP_IC_Kernel;
typedef CGAL::Surface_mesh_default_triangulation_3 							Tr;
typedef CGAL::Complex_2_in_triangulation_3<Tr> 								C2t3;
typedef CGAL_EP_IC_Kernel   												CGAL_Kernel_for_remesh;
typedef CGAL::Polyhedron_3<CGAL_Kernel_for_remesh,Customized_items>         Polyhedron;
typedef Polyhedron::HalfedgeDS            									HalfedgeDS;


int main(){
	Tr tr;            // 3D-Delaunay triangulation
  	C2t3 c2t3 (tr);   // 2D-complex in 3D-Delaunay triangulation

	CGAL::Polyhedron_3<CGAL_Kernel_for_remesh,Customized_items>  crSurface_for_remesh;
	Build_triangle<HalfedgeDS> triangle;
	crSurface_for_remesh.delegate(triangle);

	CGAL::AABB_polyhedral_oracle<CGAL::Polyhedron_3<CGAL_Kernel_for_remesh,Customized_items>, 
													CGAL_Kernel_for_remesh, 
													CGAL::Mesh_3::Robust_intersection_traits_3<CGAL_Kernel_for_remesh> > 
													AABB_input_surface(crSurface_for_remesh);

	CGAL::Surface_mesh_default_criteria_3<Tr> criteria(60,
                                                     0.2,
                                                     0.3);
	
	CGAL::make_surface_mesh(c2t3, AABB_input_surface, AABB_input_surface,
                          criteria, CGAL::Manifold_tag());

	std::ofstream out("out.off");
  	CGAL::output_surface_facets_to_off (out, c2t3);
  	std::cout << "Final number of points: " << tr.number_of_vertices() << "\n";

}
