# ==============================================================================
#  KratosShapeOptimizationApplication
#
#  License:         BSD License
#                   license: ShapeOptimizationApplication/license.txt
#
#  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
#                   Geiser Armin, https://github.com/armingeiser
#
# ==============================================================================

# Making KratosMultiphysics backward compatible with python 2.6 and 2.7
from __future__ import print_function, absolute_import, division

# importing the Kratos Library
from KratosMultiphysics import *
from KratosMultiphysics.ShapeOptimizationApplication import *

# check that KratosMultiphysics was imported in the main script
CheckForPreviousImport()

# Import algorithm base classes
from algorithm_base import OptimizationAlgorithm

# ==============================================================================
class AlgorithmSteepestDescent( OptimizationAlgorithm ) :

    # --------------------------------------------------------------------------
    def __init__( self, OptimizationSettings, MdpaController, Analyzer, Communicator ):
        self.OptimizationSettings = OptimizationSettings
        self.MdpaController = MdpaController
        self.Analyzer = Analyzer
        self.Communicator = Communicator

        self.OptimizationModelPart = MdpaController.GetOptimizationModelPart()
        self.DesignSurface = MdpaController.GetDesignSurface()

        self.maxIterations = OptimizationSettings["optimization_algorithm"]["max_iterations"].GetInt() + 1
        self.projectionOnNormalsIsSpecified = OptimizationSettings["optimization_algorithm"]["project_gradients_on_surface_normals"].GetBool()
        self.onlyObjectiveId = OptimizationSettings["objectives"][0]["identifier"].GetString()
        self.dampingIsSpecified = OptimizationSettings["design_variables"]["damping"]["perform_damping"].GetBool()

        import mapper_factory
        self.Mapper = mapper_factory.CreateMapper( MdpaController, OptimizationSettings )
        import data_logger_factory
        self.DataLogger = data_logger_factory.CreateDataLogger( MdpaController, Communicator, OptimizationSettings )

        self.GeometryUtilities = GeometryUtilities( self.DesignSurface )
        self.OptimizationUtilities = OptimizationUtilities( self.DesignSurface, OptimizationSettings )
        if self.dampingIsSpecified:
            damping_regions = self.MdpaController.GetDampingRegions()
            self.DampingUtilities = DampingUtilities( self.DesignSurface, damping_regions, self.OptimizationSettings )

    # --------------------------------------------------------------------------
    def InitializeOptimizationLoop( self ):
        self.Analyzer.InitializeBeforeOptimizationLoop()
        self.MdpaController.InitializeMeshController()
        self.DataLogger.InitializeDataLogging()

    # --------------------------------------------------------------------------
    def RunOptimizationLoop( self ):
        from custom_timer import Timer
        timer = Timer()
        timer.StartTimer()

        for self.optimizationIteration in range(1,self.maxIterations):
            print("\n>===================================================================")
            print("> ",timer.GetTimeStamp(),": Starting optimization iteration ",self.optimizationIteration)
            print(">===================================================================\n")

            timer.StartNewLap()

            self.__initializeNewShape()

            self.__analyzeShape()

            if self.projectionOnNormalsIsSpecified:
                self.__projectSensitivitiesOnSurfaceNormals()

            if self.dampingIsSpecified:
                self.__dampSensitivities()

            self.__computeShapeUpdate()

            if self.dampingIsSpecified:
                self.__dampShapeUpdate()

            self.__logCurrentOptimizationStep()

            print("\n> Time needed for current optimization step = ", timer.GetLapTime(), "s")
            print("> Time needed for total optimization so far = ", timer.GetTotalTime(), "s")

            if self.__isAlgorithmConverged():
                break
            else:
                self.__determineAbsoluteChanges()

    # --------------------------------------------------------------------------
    def FinalizeOptimizationLoop( self ):
        self.DataLogger.FinalizeDataLogging()
        self.Analyzer.FinalizeAfterOptimizationLoop()

    # --------------------------------------------------------------------------
    def __initializeNewShape( self ):
        self.MdpaController.UpdateMeshAccordingInputVariable( SHAPE_UPDATE )
        self.MdpaController.SetReferenceMeshToMesh()

    # --------------------------------------------------------------------------
    def __analyzeShape( self ):
        self.Communicator.initializeCommunication()
        self.Communicator.requestValueOf( self.onlyObjectiveId )
        self.Communicator.requestGradientOf( self.onlyObjectiveId )

        self.Analyzer.AnalyzeDesignAndReportToCommunicator( self.DesignSurface, self.optimizationIteration, self.Communicator )

        self.__storeResultOfSensitivityAnalysisOnNodes()
        self.__RevertPossibleShapeModificationsDuringAnalysis()

    # --------------------------------------------------------------------------
    def __storeResultOfSensitivityAnalysisOnNodes( self ):
        gradientOfObjectiveFunction = self.Communicator.getStandardizedGradient ( self.onlyObjectiveId )
        for nodeId, tmp_gradient in gradientOfObjectiveFunction.items():
            self.OptimizationModelPart.Nodes[nodeId].SetSolutionStepValue(OBJECTIVE_SENSITIVITY,0,tmp_gradient)

    # --------------------------------------------------------------------------
    def __RevertPossibleShapeModificationsDuringAnalysis( self ):
        self.MdpaController.SetMeshToReferenceMesh()
        self.MdpaController.SetDeformationVariablesToZero()

    # --------------------------------------------------------------------------
    def __projectSensitivitiesOnSurfaceNormals( self ):
        self.GeometryUtilities.ComputeUnitSurfaceNormals()
        self.GeometryUtilities.ProjectNodalVariableOnUnitSurfaceNormals( OBJECTIVE_SENSITIVITY )

    # --------------------------------------------------------------------------
    def __dampSensitivities( self ):
        self.DampingUtilities.DampNodalVariable( OBJECTIVE_SENSITIVITY )

    # --------------------------------------------------------------------------
    def __computeShapeUpdate( self ):
        self.__mapSensitivitiesToDesignSpace()
        self.OptimizationUtilities.ComputeSearchDirectionSteepestDescent()
        self.OptimizationUtilities.ComputeControlPointUpdate()
        self.__mapDesignUpdateToGeometrySpace()

    # --------------------------------------------------------------------------
    def __mapSensitivitiesToDesignSpace( self ):
        self.Mapper.MapToDesignSpace( OBJECTIVE_SENSITIVITY, MAPPED_OBJECTIVE_SENSITIVITY )

    # --------------------------------------------------------------------------
    def __mapDesignUpdateToGeometrySpace( self ):
        self.Mapper.MapToGeometrySpace( CONTROL_POINT_UPDATE, SHAPE_UPDATE )

    # --------------------------------------------------------------------------
    def __dampShapeUpdate( self ):
        self.DampingUtilities.DampNodalVariable( SHAPE_UPDATE )

    # --------------------------------------------------------------------------
    def __logCurrentOptimizationStep( self ):
        self.DataLogger.LogCurrentData( self.optimizationIteration )

    # --------------------------------------------------------------------------
    def __isAlgorithmConverged( self ):

        if self.optimizationIteration > 1 :

            # Check if maximum iterations were reached
            if self.optimizationIteration == self.maxIterations:
                print("\n> Maximal iterations of optimization problem reached!")
                return True

            relativeChangeOfObjectiveValue = self.DataLogger.GetValue( "RELATIVE_CHANGE_OF_OBJECTIVE_VALUE" )

            # Check for relative tolerance
            relativeTolerance = self.OptimizationSettings["optimization_algorithm"]["relative_tolerance"].GetDouble()
            if abs(relativeChangeOfObjectiveValue) < relativeTolerance:
                print("\n> Optimization problem converged within a relative objective tolerance of ",relativeTolerance,"%.")
                return True

    # --------------------------------------------------------------------------
    def __determineAbsoluteChanges( self ):
        self.OptimizationUtilities.AddFirstVariableToSecondVariable( CONTROL_POINT_UPDATE, CONTROL_POINT_CHANGE )
        self.OptimizationUtilities.AddFirstVariableToSecondVariable( SHAPE_UPDATE, SHAPE_CHANGE )


# ==============================================================================
