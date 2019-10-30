from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics.CoSimulationApplication as KMC

# Importing the base class
from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_solver_wrapper import CoSimulationSolverWrapper

# Other imports
from .sdof_solver import SDoFSolver

def Create(settings, solver_name):
    return SdofSolverWrapper(settings, solver_name)

class SdofSolverWrapper(CoSimulationSolverWrapper):
    """This class serves as basis for
    """
    def __init__(self, settings, solver_name):
        super(SdofSolverWrapper, self).__init__(settings, solver_name)

        input_file_name = self.settings["solver_wrapper_settings"]["input_file"].GetString()

        #current_model = KM.Model()
        self.mp = self.model.CreateModelPart("Sdof")

        # this creates
        self._sdof_solver = SDoFSolver(input_file_name)

    def Initialize(self):
        self._sdof_solver.Initialize()

    def OutputSolutionStep(self):
        self._sdof_solver.OutputSolutionStep()

    def AdvanceInTime(self, current_time):
        return self._sdof_solver.AdvanceInTime(current_time)


    def SolveSolutionStep(self):
        self._sdof_solver.SetSolutionStepValue("ROOT_POINT_DISPLACEMENT", self.mp[KMC.SCALAR_ROOT_POINT_DISPLACEMENT], 0)
        self._sdof_solver.SetSolutionStepValue("LOAD", self.mp[KMC.SCALAR_FORCE], 0)
        self._sdof_solver.SolveSolutionStep()
        self.mp[KMC.SCALAR_DISPLACEMENT] = self._sdof_solver.GetSolutionStepValue("DISPLACEMENT", 0)
        self.mp[KMC.SCALAR_REACTION] = self._sdof_solver.GetSolutionStepValue("REACTION", 0)
