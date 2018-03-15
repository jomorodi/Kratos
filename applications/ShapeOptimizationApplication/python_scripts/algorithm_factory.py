# ==============================================================================
#  KratosShapeOptimizationApplication
#
#  License:         BSD License
#                   license: ShapeOptimizationApplication/license.txt
#
#  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
#
# ==============================================================================

# Making KratosMultiphysics backward compatible with python 2.6 and 2.7
from __future__ import print_function, absolute_import, division

# importing the Kratos Library
from KratosMultiphysics import *
from KratosMultiphysics.ShapeOptimizationApplication import *

# check that KratosMultiphysics was imported in the main script
CheckForPreviousImport()

# Additional imports
from algorithm_steepest_descent import AlgorithmSteepestDescent
from algorithm_penalized_projection import AlgorithmPenalizedProjection

# ==============================================================================
def CreateAlgorithm( optimization_settings, mdpa_controller, analyzer, communicator ):
    algorithm_name = optimization_settings["optimization_algorithm"]["name"].GetString()

    if algorithm_name == "steepest_descent":
        return AlgorithmSteepestDescent( optimization_settings,
                                         mdpa_controller,
                                         analyzer,
                                         communicator )
    elif algorithm_name == "penalized_projection":
        return AlgorithmPenalizedProjection( optimization_settings,
                                             mdpa_controller,
                                             analyzer,
                                             communicator )
    else:
        raise NameError("The following optimization algorithm not supported by the algorithm factory: " + AlgorithmName)

# # ==============================================================================