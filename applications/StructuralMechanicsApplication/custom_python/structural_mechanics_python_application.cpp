// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Riccardo Rossi
//

// System includes

#if defined(KRATOS_PYTHON)
// External includes
#include <boost/python.hpp>


// Project includes
#include "includes/define.h"
#include "structural_mechanics_application_variables.h"
#include "structural_mechanics_application.h"
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_python/add_custom_constitutive_laws_to_python.h"
#include "custom_python/add_cross_sections_to_python.h"

namespace Kratos
{

namespace Python
{

using namespace boost::python;



BOOST_PYTHON_MODULE(KratosStructuralMechanicsApplication)
{

    class_<KratosStructuralMechanicsApplication,
            KratosStructuralMechanicsApplication::Pointer,
            bases<KratosApplication>, boost::noncopyable >("KratosStructuralMechanicsApplication")
            ;

    AddCustomStrategiesToPython();
    AddCustomProcessesToPython();
    AddCustomUtilitiesToPython();
    AddCustomConstitutiveLawsToPython();
    AddCrossSectionsToPython();

    //registering variables in python
    // Generalized eigenvalue problem
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EIGENVALUE_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( EIGENVECTOR_MATRIX )

    // cross section
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION_OUTPUT_PLY_ID )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CROSS_SECTION_OUTPUT_PLY_LOCATION )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_LAYERS)

	// Geometrical
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(LOCAL_ELEMENT_ORIENTATION)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(MATERIAL_ORIENTATION_ANGLE)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(LOCAL_MATERIAL_ORIENTATION_VECTOR_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(LUMPED_MASS_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(CONDENSED_DOF_LIST)

    // Truss generalized variables
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(TRUSS_PRESTRESS_PK2)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(TRUSS_IS_CABLE)

    // Beam generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(AREA_EFFECTIVE_Y)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(AREA_EFFECTIVE_Z)    
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(INERTIA_ROT_Y)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(INERTIA_ROT_Z)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(LOCAL_AXES_VECTOR)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(TORSIONAL_INERTIA)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(I22)
	KRATOS_REGISTER_IN_PYTHON_VARIABLE(I33)

    // shell generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( STENBERG_SHEAR_STABILIZATION_SUITABLE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRAIN )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRAIN_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CURVATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_CURVATURE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_FORCE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_FORCE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_MOMENT_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_TOP_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_TOP_SURFACE_GLOBAL )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_MIDDLE_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_MIDDLE_SURFACE_GLOBAL )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_BOTTOM_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_STRESS_BOTTOM_SURFACE_GLOBAL )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( VON_MISES_STRESS_TOP_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( VON_MISES_STRESS_MIDDLE_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( VON_MISES_STRESS_BOTTOM_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE_GLOBAL )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE_GLOBAL )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_4PLY_THROUGH_THICKNESS )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( TSAI_WU_RESERVE_FACTOR )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ORTHOTROPIC_LAMINA_STRENGTHS )

    // Shell energies
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_MEMBRANE_ENERGY )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_BENDING_ENERGY )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_SHEAR_ENERGY )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_MEMBRANE_ENERGY_FRACTION )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_BENDING_ENERGY_FRACTION )
	KRATOS_REGISTER_IN_PYTHON_VARIABLE( SHELL_ELEMENT_SHEAR_ENERGY_FRACTION )

    // Membrane variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( MEMBRANE_PRESTRESS )
    
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( NODAL_STIFFNESS )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( NODAL_DAMPING_RATIO )

    // CONDITIONS
    /* Point moment */
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( POINT_MOMENT )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( CROSS_AREA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( MEAN_RADIUS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SECTION_SIDES )

    /* Adding the SPRISM EAS variables */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(ALPHA_EAS);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(EAS_IMP);

    /* Adding the SPRISM additional variables */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(ANG_ROT);

    /* Adding the SPRISM number of transversal integration points */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(NINT_TRANS);

    /* Adding the SPRISM variable to deactivate the quadratic interpolation */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(QUAD_ON);

    /* Hencky strain */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(HENCKY_STRAIN_VECTOR);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(HENCKY_STRAIN_TENSOR);

    // Adding the LAMBDA load factor considered in the arclength
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(LAMBDA);

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( VON_MISES_STRESS ) 
    
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(REFERENCE_DEFORMATION_GRADIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(REFERENCE_DEFORMATION_GRADIENT_DETERMINANT);
    
    // Rayleigh variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( RAYLEIGH_ALPHA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( RAYLEIGH_BETA )

    // System damping
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( SYSTEM_DAMPING_RATIO )

    //nodal load variables
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(  POINT_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(  LINE_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(  SURFACE_LOAD )

    //condition load variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(POINT_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(LINE_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(SURFACE_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(POSITIVE_FACE_PRESSURES_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(NEGATIVE_FACE_PRESSURES_VECTOR )
}


}  // namespace Python.

}  // namespace Kratos.

#endif // KRATOS_PYTHON defined
