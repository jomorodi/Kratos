//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//


// System includes

#if defined(KRATOS_PYTHON)
// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define.h"
#include "rans_application.h"
#include "rans_application_variables.h"
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"


namespace Kratos {
namespace Python {

PYBIND11_MODULE(KratosRANSApplication,m)
{
    namespace py = pybind11;

    py::class_<KratosRANSApplication,
        KratosRANSApplication::Pointer,
        KratosApplication>(m, "KratosRANSApplication")
        .def(py::init<>())
        ;

    AddCustomStrategiesToPython(m);
    AddCustomUtilitiesToPython(m);
    AddCustomProcessesToPython(m);

    // incompressible potential flow specific variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VELOCITY_POTENTIAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESSURE_POTENTIAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_IS_INLET )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_IS_OUTLET )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_IS_STRUCTURE )

    // residual based flux corrected stabilization variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_STABILIZATION_DISCRETE_UPWIND_OPERATOR_COEFFICIENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_STABILIZATION_DIAGONAL_POSITIVITY_PRESERVING_COEFFICIENT )

    // algebraic flux corrected stabilization variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AFC_POSITIVE_ANTI_DIFFUSIVE_FLUX )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AFC_NEGATIVE_ANTI_DIFFUSIVE_FLUX )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AFC_POSITIVE_ANTI_DIFFUSIVE_FLUX_LIMIT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AFC_NEGATIVE_ANTI_DIFFUSIVE_FLUX_LIMIT )

    // k-epsilon-high-re turbulence modelling variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY_RATE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_AUXILIARY_VARIABLE_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RANS_AUXILIARY_VARIABLE_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_KINETIC_ENERGY_SIGMA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C_MU )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TURBULENCE_RANS_C2 )
}

} // namespace Python.
} // namespace Kratos.

#endif // KRATOS_PYTHON defined
