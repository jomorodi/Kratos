//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		BSD License
//					Kratos default license: kratos/license.txt
//
//  Main authors:    Ilaria Iaconeta, Bodhinanda Chandra
//


// System includes
#include <omp.h>
#include <sstream>

// External includes

// Project includes
#include "includes/define.h"
#include "custom_elements/updated_lagrangian_quadrilateral.hpp"
#include "utilities/math_utils.h"
#include "includes/constitutive_law.h"
#include "particle_mechanics_application_variables.h"
#include "includes/checks.h"
#include "custom_utilities/mpm_energy_calculation_utility.h"
#include "custom_utilities/mpm_explicit_utilities.h"

namespace Kratos
{

/**
 * Flags related to the element computation
 */
KRATOS_CREATE_LOCAL_FLAG( UpdatedLagrangianQuadrilateral, COMPUTE_RHS_VECTOR,                 0 );
KRATOS_CREATE_LOCAL_FLAG( UpdatedLagrangianQuadrilateral, COMPUTE_LHS_MATRIX,                 1 );
KRATOS_CREATE_LOCAL_FLAG( UpdatedLagrangianQuadrilateral, COMPUTE_RHS_VECTOR_WITH_COMPONENTS, 2 );
KRATOS_CREATE_LOCAL_FLAG( UpdatedLagrangianQuadrilateral, COMPUTE_LHS_MATRIX_WITH_COMPONENTS, 3 );

//******************************CONSTRUCTOR*******************************************
//************************************************************************************

UpdatedLagrangianQuadrilateral::UpdatedLagrangianQuadrilateral( )
    : Element( )
    , mMP()
{
    //DO NOT CALL IT: only needed for Register and Serialization!!!
}
//******************************CONSTRUCTOR*******************************************
//************************************************************************************
UpdatedLagrangianQuadrilateral::UpdatedLagrangianQuadrilateral( IndexType NewId, GeometryType::Pointer pGeometry )
    : Element( NewId, pGeometry )
    , mMP()
{
    //DO NOT ADD DOFS HERE!!!
}

//******************************CONSTRUCTOR*******************************************
//************************************************************************************

UpdatedLagrangianQuadrilateral::UpdatedLagrangianQuadrilateral( IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties )
    : Element( NewId, pGeometry, pProperties )
    , mMP()
{
    mFinalizedStep = true;


}
//******************************COPY CONSTRUCTOR**************************************
//************************************************************************************

UpdatedLagrangianQuadrilateral::UpdatedLagrangianQuadrilateral( UpdatedLagrangianQuadrilateral const& rOther)
    :Element(rOther)
    , mMP(rOther.mMP)
    , mDeformationGradientF0(rOther.mDeformationGradientF0)
    , mDeterminantF0(rOther.mDeterminantF0)
    , mConstitutiveLawVector(rOther.mConstitutiveLawVector)
    , mFinalizedStep(rOther.mFinalizedStep)
{
}

//*******************************ASSIGMENT OPERATOR***********************************
//************************************************************************************

UpdatedLagrangianQuadrilateral&  UpdatedLagrangianQuadrilateral::operator=(UpdatedLagrangianQuadrilateral const& rOther)
{
    Element::operator=(rOther);

    mMP = rOther.mMP;

    mDeformationGradientF0.clear();
    mDeformationGradientF0 = rOther.mDeformationGradientF0;

    mDeterminantF0 = rOther.mDeterminantF0;
    mConstitutiveLawVector = rOther.mConstitutiveLawVector;

    return *this;
}

//*********************************OPERATIONS*****************************************
//************************************************************************************

Element::Pointer UpdatedLagrangianQuadrilateral::Create( IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties ) const
{
    return Element::Pointer( new UpdatedLagrangianQuadrilateral( NewId, GetGeometry().Create( ThisNodes ), pProperties ) );
}

//************************************CLONE*******************************************
//************************************************************************************

Element::Pointer UpdatedLagrangianQuadrilateral::Clone( IndexType NewId, NodesArrayType const& rThisNodes ) const
{
    UpdatedLagrangianQuadrilateral NewElement (NewId, GetGeometry().Create( rThisNodes ), pGetProperties() );

    NewElement.mMP = mMP;

    NewElement.mConstitutiveLawVector = mConstitutiveLawVector->Clone();

    NewElement.mDeformationGradientF0 = mDeformationGradientF0;

    NewElement.mDeterminantF0 = mDeterminantF0;

    return Element::Pointer( new UpdatedLagrangianQuadrilateral(NewElement) );
}

//*******************************DESTRUCTOR*******************************************
//************************************************************************************
UpdatedLagrangianQuadrilateral::~UpdatedLagrangianQuadrilateral()
{
}


//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::Initialize()
{
    KRATOS_TRY

    // Initialize parameters
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    mDeterminantF0 = 1;
    mDeformationGradientF0 = IdentityMatrix(dimension);

    // Initialize constitutive law and materials
    InitializeMaterial();

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::InitializeGeneralVariables (GeneralVariables& rVariables, const ProcessInfo& rCurrentProcessInfo)
{
    const unsigned int number_of_nodes = GetGeometry().size();
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    unsigned int voigtsize  = 3;

    if( dimension == 3 )
    {
        voigtsize  = 6;
    }

    rVariables.detF  = 1;

    rVariables.detF0 = 1;

    rVariables.detFT = 1;

    rVariables.B.resize( voigtsize, number_of_nodes * dimension, false );

    rVariables.F.resize( dimension, dimension, false );

    rVariables.F0.resize( dimension, dimension, false );

    rVariables.FT.resize( dimension, dimension, false );

    rVariables.ConstitutiveMatrix.resize( voigtsize, voigtsize, false );

    rVariables.StrainVector.resize( voigtsize, false );

    rVariables.StressVector.resize( voigtsize, false );

    rVariables.DN_DX.resize( number_of_nodes, dimension, false );
    rVariables.DN_De.resize( number_of_nodes, dimension, false );

    rVariables.N = this->MPMShapeFunctionPointValues(rVariables.N, mMP.xg);

    // Reading shape functions local gradients
    rVariables.DN_De = this->MPMShapeFunctionsLocalGradients( rVariables.DN_De, mMP.xg);

    // CurrentDisp is the variable unknown. It represents the nodal delta displacement. When it is predicted is equal to zero.
    rVariables.CurrentDisp = CalculateCurrentDisp(rVariables.CurrentDisp, rCurrentProcessInfo);
}
//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::SetGeneralVariables(GeneralVariables& rVariables,
        ConstitutiveLaw::Parameters& rValues)
{
    // Variables.detF is the determinant of the incremental total deformation gradient
    rVariables.detF  = MathUtils<double>::Det(rVariables.F);

    // Check if detF is negative (element is inverted)
    if(rVariables.detF<0)
    {
        KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" Element: "<<this->Id()<<std::endl;
        KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" Element position "<< mMP.xg <<std::endl;
        const unsigned int number_of_nodes = GetGeometry().PointsNumber();

        for ( unsigned int i = 0; i < number_of_nodes; i++ )
        {
            const array_1d<double, 3> & current_position      = GetGeometry()[i].Coordinates();
            const array_1d<double, 3> & current_displacement  = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);
            const array_1d<double, 3> & previous_displacement = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT,1);
            //array_1d<double, 3> PreviousPosition  = current_position - (current_displacement-previous_displacement);

            KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" NODE ["<<GetGeometry()[i].Id()<<"]: (Current position: "<<current_position<<") "<<std::endl;
            KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" ---Current Disp: "<<current_displacement<<" (Previous Disp: "<<previous_displacement<<")"<<std::endl;
        }

        for ( unsigned int i = 0; i < number_of_nodes; i++ )
        {
            if( GetGeometry()[i].SolutionStepsDataHas(CONTACT_FORCE) )
            {
                const array_1d<double, 3 > & PreContactForce = GetGeometry()[i].FastGetSolutionStepValue(CONTACT_FORCE,1);
                const array_1d<double, 3 > & ContactForce = GetGeometry()[i].FastGetSolutionStepValue(CONTACT_FORCE);
                KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" ---Contact_Force: (Pre:"<<PreContactForce<<", Current:"<<ContactForce<<") "<<std::endl;
            }
            else
            {
                KRATOS_INFO("UpdatedLagrangianQuadrilateral")<<" ---Contact_Force: NULL "<<std::endl;
            }
        }

        KRATOS_ERROR << "MPM UPDATED LAGRANGIAN DISPLACEMENT ELEMENT INVERTED: |F|<0  detF = " << rVariables.detF << std::endl;
    }

    rVariables.detFT = rVariables.detF * rVariables.detF0;
    rVariables.FT    = prod( rVariables.F, rVariables.F0 );

    rValues.SetDeterminantF(rVariables.detFT);
    rValues.SetDeformationGradientF(rVariables.FT);
    rValues.SetStrainVector(rVariables.StrainVector);
    rValues.SetStressVector(rVariables.StressVector);
    rValues.SetConstitutiveMatrix(rVariables.ConstitutiveMatrix);
    rValues.SetShapeFunctionsDerivatives(rVariables.DN_DX);
    rValues.SetShapeFunctionsValues(rVariables.N);

}


//************************************************************************************
//*****************check size of LHS and RHS matrices*********************************

void UpdatedLagrangianQuadrilateral::InitializeSystemMatrices(MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        Flags& rCalculationFlags)
{
    const unsigned int number_of_nodes = GetGeometry().size();
    const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

    // Resizing the LHS matrix if needed
    unsigned int matrix_size = number_of_nodes * dimension;   //number of degrees of freedom

    if ( rCalculationFlags.Is(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX) ) //calculation of the matrix is required
    {
        if ( rLeftHandSideMatrix.size1() != matrix_size )
            rLeftHandSideMatrix.resize( matrix_size, matrix_size, false );

        noalias( rLeftHandSideMatrix ) = ZeroMatrix(matrix_size, matrix_size); //resetting LHS
    }

    // Resizing the RHS vector if needed
    if ( rCalculationFlags.Is(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR) ) //calculation of the matrix is required
    {
        if ( rRightHandSideVector.size() != matrix_size )
            rRightHandSideVector.resize( matrix_size, false );

        rRightHandSideVector = ZeroVector( matrix_size ); //resetting RHS
    }
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateElementalSystem( LocalSystemComponents& rLocalSystem,
        ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    // Create and initialize element variables:
    GeneralVariables Variables;
    this->InitializeGeneralVariables(Variables,rCurrentProcessInfo);
    const bool is_explicit = (rCurrentProcessInfo.Has(IS_EXPLICIT))
        ? rCurrentProcessInfo.GetValue(IS_EXPLICIT)
        : false;

    // Create constitutive law parameters:
    ConstitutiveLaw::Parameters Values(GetGeometry(),GetProperties(),rCurrentProcessInfo);

    // Set constitutive law flags:
    Flags &ConstitutiveLawOptions=Values.GetOptions();
    ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

    if (!is_explicit)
    {
        ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN);
        ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);

        // Compute element kinematics B, F, DN_DX ...
        this->CalculateKinematics(Variables, rCurrentProcessInfo);

        // Set general variables to constitutivelaw parameters
        this->SetGeneralVariables(Variables, Values);

        // Calculate Material Response
        /* NOTE:
        The function below will call CalculateMaterialResponseCauchy() by default and then (may)
        call CalculateMaterialResponseKirchhoff() in the constitutive_law.*/
        mConstitutiveLawVector->CalculateMaterialResponse(Values, Variables.StressMeasure);

        /* NOTE:
        The material points will have constant mass as defined at the beginning.
        However, the density and volume (integration weight) are changing every time step.*/
        // Update MP_density
        mMP.density = (GetProperties()[DENSITY]) / Variables.detFT;
    }
    else if (is_explicit)
    {
        rLocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX, false);
    }
    
    // The MP_volume (integration weight) is evaluated
    mMP.volume = mMP.mass / mMP.density;

    if ( rLocalSystem.CalculationFlags.Is(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX) ) // if calculation of the matrix is required
    {
        // Contributions to stiffness matrix calculated on the reference configuration
        this->CalculateAndAddLHS ( rLocalSystem, Variables, mMP.volume, rCurrentProcessInfo);
    }

    if ( rLocalSystem.CalculationFlags.Is(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR) ) // if calculation of the vector is required
    {
        // Contribution to forces (in residual term) are calculated
        Vector volume_force = mMP.volume_acceleration * mMP.mass;
        this->CalculateAndAddRHS ( rLocalSystem, Variables, volume_force, mMP.volume, rCurrentProcessInfo);
    }

    KRATOS_CATCH( "" )
}
//*********************************COMPUTE KINEMATICS*********************************
//************************************************************************************


void UpdatedLagrangianQuadrilateral::CalculateKinematics(GeneralVariables& rVariables, ProcessInfo& rCurrentProcessInfo)

{
    KRATOS_TRY

    // Define the stress measure
    rVariables.StressMeasure = ConstitutiveLaw::StressMeasure_Cauchy;

    // Calculating the reference jacobian from cartesian coordinates to parent coordinates for the MP element [dx_n/d£]
    Matrix Jacobian;
    Jacobian = this->MPMJacobian( Jacobian, mMP.xg);

    // Calculating the inverse of the jacobian and the parameters needed [d£/dx_n]
    Matrix InvJ;
    double detJ;
    MathUtils<double>::InvertMatrix( Jacobian, InvJ, detJ);

    // Calculating the current jacobian from cartesian coordinates to parent coordinates for the MP element [dx_n+1/d£]
    Matrix jacobian;
    jacobian = this->MPMJacobianDelta( jacobian, mMP.xg, rVariables.CurrentDisp);

    // Calculating the inverse of the jacobian and the parameters needed [d£/(dx_n+1)]
    Matrix Invj;
    MathUtils<double>::InvertMatrix( jacobian, Invj, detJ); //overwrites detJ

    // Compute cartesian derivatives [dN/dx_n+1]
    rVariables.DN_DX = prod( rVariables.DN_De, Invj); //overwrites DX now is the current position dx

    /* NOTE::
    Deformation Gradient F [(dx_n+1 - dx_n)/dx_n] is to be updated in constitutive law parameter as total deformation gradient.
    The increment of total deformation gradient can be evaluated in 2 ways, which are:
    1. By: noalias( rVariables.F ) = prod( jacobian, InvJ);
    2. By means of the gradient of nodal displacement: using this second expression quadratic convergence is not guarantee

    (NOTICE: Here, we are using method no. 2)*/

    // METHOD 1: Update Deformation gradient: F [dx_n+1/dx_n] = [dx_n+1/d£] [d£/dx_n]
    // noalias( rVariables.F ) = prod( jacobian, InvJ);

    // METHOD 2: Update Deformation gradient: F_ij = δ_ij + u_i,j
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    Matrix I = IdentityMatrix(dimension);
    Matrix gradient_displacement = ZeroMatrix(dimension, dimension);
    rVariables.CurrentDisp = CalculateCurrentDisp(rVariables.CurrentDisp, rCurrentProcessInfo);
    gradient_displacement = prod(trans(rVariables.CurrentDisp),rVariables.DN_DX);

    noalias( rVariables.F ) = (I + gradient_displacement);

    // Determinant of the previous Deformation Gradient F_n
    rVariables.detF0 = mDeterminantF0;
    rVariables.F0    = mDeformationGradientF0;

    // Compute the deformation matrix B
    this->CalculateDeformationMatrix(rVariables.B, rVariables.F, rVariables.DN_DX);


    KRATOS_CATCH( "" )
}
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateDeformationMatrix(Matrix& rB,
        Matrix& rF,
        Matrix& rDN_DX)
{
    KRATOS_TRY

    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

    rB.clear(); //set all components to zero

    if( dimension == 2 )
    {
        for ( unsigned int i = 0; i < number_of_nodes; i++ )
        {
            unsigned int index = 2 * i;
            rB( 0, index + 0 ) = rDN_DX( i, 0 );
            rB( 1, index + 1 ) = rDN_DX( i, 1 );
            rB( 2, index + 0 ) = rDN_DX( i, 1 );
            rB( 2, index + 1 ) = rDN_DX( i, 0 );
        }
    }
    else if( dimension == 3 )
    {
        for ( unsigned int i = 0; i < number_of_nodes; i++ )
        {
            unsigned int index = 3 * i;

            rB( 0, index + 0 ) = rDN_DX( i, 0 );
            rB( 1, index + 1 ) = rDN_DX( i, 1 );
            rB( 2, index + 2 ) = rDN_DX( i, 2 );

            rB( 3, index + 0 ) = rDN_DX( i, 1 );
            rB( 3, index + 1 ) = rDN_DX( i, 0 );

            rB( 4, index + 1 ) = rDN_DX( i, 2 );
            rB( 4, index + 2 ) = rDN_DX( i, 1 );

            rB( 5, index + 0 ) = rDN_DX( i, 2 );
            rB( 5, index + 2 ) = rDN_DX( i, 0 );
        }
    }
    else
    {
        KRATOS_ERROR << "Dimension given is wrong!" << std::endl;
    }

    KRATOS_CATCH( "" )
}
//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateAndAddRHS(
    LocalSystemComponents& rLocalSystem, 
    GeneralVariables& rVariables, 
    Vector& rVolumeForce, 
    const double& rIntegrationWeight,
    const ProcessInfo& rCurrentProcessInfo)
{
    // Contribution of the internal and external forces
    if( rLocalSystem.CalculationFlags.Is( UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR_WITH_COMPONENTS ) )
    {
        std::vector<VectorType>& rRightHandSideVectors = rLocalSystem.GetRightHandSideVectors();
        const std::vector< Variable< VectorType > >& rRightHandSideVariables = rLocalSystem.GetRightHandSideVariables();
        for( unsigned int i=0; i<rRightHandSideVariables.size(); i++ )
        {
            bool calculated = false;
            if( rRightHandSideVariables[i] == EXTERNAL_FORCES_VECTOR )
            {
                // Operation performed: rRightHandSideVector += ExtForce*IntToReferenceWeight
                this->CalculateAndAddExternalForces( rRightHandSideVectors[i], rVariables, rVolumeForce, rIntegrationWeight );
                calculated = true;
            }

            if( rRightHandSideVariables[i] == INTERNAL_FORCES_VECTOR )
            {
                // Operation performed: rRightHandSideVector -= IntForce*IntToReferenceWeight
                this->CalculateAndAddInternalForces( rRightHandSideVectors[i], rVariables, rIntegrationWeight );
                calculated = true;
            }

            KRATOS_ERROR_IF(calculated == false) << "ELEMENT can not supply the required local system variable: " << rRightHandSideVariables[i] << std::endl;

        }
    }
    else
    {
        VectorType& rRightHandSideVector = rLocalSystem.GetRightHandSideVector();

        // Operation performed: rRightHandSideVector += ExtForce*IntToReferenceWeight
        this->CalculateAndAddExternalForces( rRightHandSideVector, rVariables, rVolumeForce, rIntegrationWeight );
        const bool is_explicit = (rCurrentProcessInfo.Has(IS_EXPLICIT))
            ? rCurrentProcessInfo.GetValue(IS_EXPLICIT)
            : false;

        if (is_explicit)
        {
            this->MPMShapeFunctionPointValues(rVariables.N, mMP.xg);
            Matrix Jacobian;
            Jacobian = this->MPMJacobian(Jacobian, mMP.xg);
            Matrix InvJ;
            double detJ;
            MathUtils<double>::InvertMatrix(Jacobian, InvJ, detJ);
            Matrix DN_De;
            this->MPMShapeFunctionsLocalGradients(DN_De, mMP.xg); // parametric gradients
            rVariables.DN_DX = prod(DN_De, InvJ); // cartesian gradients

            MPMExplicitUtilities::CalculateAndAddExplicitInternalForce(*this,
                rVariables.DN_DX, mMP.cauchy_stress_vector, mMP.volume,
                mConstitutiveLawVector->GetStrainSize(), rRightHandSideVector);
        }
        else
        {
            // Operation performed: rRightHandSideVector -= IntForce*IntToReferenceWeight
            this->CalculateAndAddInternalForces(rRightHandSideVector, rVariables, rIntegrationWeight);
        }
    }

}
//************************************************************************************
//*********************Calculate the contribution of external force*******************

void UpdatedLagrangianQuadrilateral::CalculateAndAddExternalForces(VectorType& rRightHandSideVector,
        GeneralVariables& rVariables,
        Vector& rVolumeForce,
        const double& rIntegrationWeight)

{
    KRATOS_TRY

    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        int index = dimension * i;

        for ( unsigned int j = 0; j < dimension; j++ )
        {
            rRightHandSideVector[index + j] += rVariables.N[i] * rVolumeForce[j];
        }
    }

    KRATOS_CATCH( "" )
}
//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateAndAddInternalForces(VectorType& rRightHandSideVector,
        GeneralVariables & rVariables,
        const double& rIntegrationWeight)
{
    KRATOS_TRY

    VectorType internal_forces = rIntegrationWeight * prod( trans( rVariables.B ), rVariables.StressVector );
    noalias( rRightHandSideVector ) -= internal_forces;

    KRATOS_CATCH( "" )
}
//************************************************************************************
//************************************************************************************


void UpdatedLagrangianQuadrilateral::CalculateExplicitStresses(const ProcessInfo& rCurrentProcessInfo, GeneralVariables& rVariables)
{
    KRATOS_TRY

    // Create constitutive law parameters:
    ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);

    // Define the stress measure
    rVariables.StressMeasure = ConstitutiveLaw::StressMeasure_Cauchy;

    // Set constitutive law flags:
    Flags& ConstitutiveLawOptions = Values.GetOptions();
    ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS, true);

    // use element provided strain incremented from velocity gradient
    ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, true);
    ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, false);


    // Compute explicit element kinematics, strain is incremented here.
    this->MPMShapeFunctionPointValues(rVariables.N, mMP.xg);
    Matrix Jacobian;
    Jacobian = this->MPMJacobian(Jacobian, mMP.xg);
    Matrix InvJ;
    double detJ;
    MathUtils<double>::InvertMatrix(Jacobian, InvJ, detJ);
    Matrix DN_De;
    this->MPMShapeFunctionsLocalGradients(DN_De, mMP.xg); // parametric gradients
    rVariables.DN_DX = prod(DN_De, InvJ); // cartesian gradients
    MPMExplicitUtilities::CalculateExplicitKinematics(rCurrentProcessInfo, *this, rVariables.DN_DX,
        mMP.almansi_strain_vector, rVariables.F, mConstitutiveLawVector->GetStrainSize());
    rVariables.StressVector = mMP.cauchy_stress_vector;
    rVariables.StrainVector = mMP.almansi_strain_vector;

    // Update gradient deformation
    rVariables.F0 = mDeformationGradientF0; // total member def grad NOT including this increment    
    rVariables.FT = prod(rVariables.F, rVariables.F0); // total def grad including this increment    
    rVariables.detF = MathUtils<double>::Det(rVariables.F); // det of current increment
    rVariables.detF0 = MathUtils<double>::Det(rVariables.F0); // det of def grad NOT including this increment
    rVariables.detFT = MathUtils<double>::Det(rVariables.FT); // det of total def grad including this increment
    mDeformationGradientF0 = rVariables.FT; // update member internal total grad def
    mDeterminantF0 = rVariables.detFT; // update member internal total grad def det

    // Update MP volume
    if (rCurrentProcessInfo.GetValue(IS_COMPRESSIBLE))
    {
        mMP.density = (GetProperties()[DENSITY]) / rVariables.detFT;
        mMP.volume = mMP.mass / mMP.density;
    }

    rVariables.CurrentDisp = CalculateCurrentDisp(rVariables.CurrentDisp, rCurrentProcessInfo);

    // Set general variables to constitutivelaw parameters
    this->SetGeneralVariables(rVariables, Values);

    // Calculate Material Response
    /* NOTE:
    The function below will call CalculateMaterialResponseCauchy() by default and then (may)
    call CalculateMaterialResponseKirchhoff() in the constitutive_law.*/
    mConstitutiveLawVector->CalculateMaterialResponse(Values, rVariables.StressMeasure);

    KRATOS_CATCH("")
}
//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateAndAddLHS(LocalSystemComponents& rLocalSystem, GeneralVariables& rVariables, const double& rIntegrationWeight, const ProcessInfo& rCurrentProcessInfo)
{
    // Contributions of the stiffness matrix calculated on the reference configuration
    if( rLocalSystem.CalculationFlags.Is( UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX_WITH_COMPONENTS ) )
    {
        std::vector<MatrixType>& rLeftHandSideMatrices = rLocalSystem.GetLeftHandSideMatrices();
        const std::vector< Variable< MatrixType > >& rLeftHandSideVariables = rLocalSystem.GetLeftHandSideVariables();

        for( unsigned int i=0; i<rLeftHandSideVariables.size(); i++ )
        {
            bool calculated = false;
            if( rLeftHandSideVariables[i] == MATERIAL_STIFFNESS_MATRIX )
            {
                // Operation performed: add Km to the rLefsHandSideMatrix
                this->CalculateAndAddKuum( rLeftHandSideMatrices[i], rVariables, rIntegrationWeight );
                calculated = true;
            }

            if( rLeftHandSideVariables[i] == GEOMETRIC_STIFFNESS_MATRIX && 
                !rCurrentProcessInfo.Has(IGNORE_GEOMETRIC_STIFFNESS))
            {
                // Operation performed: add Kg to the rLefsHandSideMatrix
                this->CalculateAndAddKuug( rLeftHandSideMatrices[i], rVariables, rIntegrationWeight );
                calculated = true;
            }

            KRATOS_ERROR_IF(calculated == false) << "ELEMENT can not supply the required local system variable: " << rLeftHandSideVariables[i] << std::endl;
        }
    }
    else
    {
        MatrixType& rLeftHandSideMatrix = rLocalSystem.GetLeftHandSideMatrix();

        // Operation performed: add K_material to the rLefsHandSideMatrix
        this->CalculateAndAddKuum( rLeftHandSideMatrix, rVariables, rIntegrationWeight );

        if (!rCurrentProcessInfo.Has(IGNORE_GEOMETRIC_STIFFNESS))
        {
            // Operation performed: add K_geometry to the rLefsHandSideMatrix
            this->CalculateAndAddKuug(rLeftHandSideMatrix, rVariables, rIntegrationWeight);
        }
    }
}
//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateAndAddKuum(MatrixType& rLeftHandSideMatrix,
        GeneralVariables& rVariables,
        const double& rIntegrationWeight
                                                        )
{
    KRATOS_TRY

    noalias( rLeftHandSideMatrix ) += prod( trans( rVariables.B ),  rIntegrationWeight * Matrix( prod( rVariables.ConstitutiveMatrix, rVariables.B ) ) );

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateAndAddKuug(MatrixType& rLeftHandSideMatrix,
        GeneralVariables& rVariables,
        const double& rIntegrationWeight)

{
    KRATOS_TRY

    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    Matrix stress_tensor = MathUtils<double>::StressVectorToTensor( rVariables.StressVector );
    Matrix reduced_Kg    = prod( rVariables.DN_DX, rIntegrationWeight * Matrix( prod( stress_tensor, trans( rVariables.DN_DX ) ) ) );
    MathUtils<double>::ExpandAndAddReducedMatrix( rLeftHandSideMatrix, reduced_Kg, dimension );

    KRATOS_CATCH( "" )
}
//************************************CALCULATE VOLUME CHANGE*************************
//************************************************************************************

double& UpdatedLagrangianQuadrilateral::CalculateVolumeChange( double& rVolumeChange, GeneralVariables& rVariables )
{
    KRATOS_TRY

    rVolumeChange = 1.0 / (rVariables.detF * rVariables.detF0);

    return rVolumeChange;

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************
void UpdatedLagrangianQuadrilateral::CalculateRightHandSide( VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo )
{
    // Create local system components
    LocalSystemComponents LocalSystem;

    // Set calculation flags
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR);

    MatrixType LeftHandSideMatrix = Matrix();

    // Initialize sizes for the system components:
    this->InitializeSystemMatrices( LeftHandSideMatrix, rRightHandSideVector, LocalSystem.CalculationFlags );

    // Set Variables to Local system components
    LocalSystem.SetLeftHandSideMatrix(LeftHandSideMatrix);
    LocalSystem.SetRightHandSideVector(rRightHandSideVector);

    // Calculate elemental system
    CalculateElementalSystem( LocalSystem, rCurrentProcessInfo );
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateRightHandSide( std::vector< VectorType >& rRightHandSideVectors, const std::vector< Variable< VectorType > >& rRHSVariables, ProcessInfo& rCurrentProcessInfo )
{
    // Create local system components
    LocalSystemComponents LocalSystem;

    // Set calculation flags
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR);
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR_WITH_COMPONENTS);

    MatrixType LeftHandSideMatrix = Matrix();

    //Initialize sizes for the system components:
    if( rRHSVariables.size() != rRightHandSideVectors.size() )
        rRightHandSideVectors.resize(rRHSVariables.size());

    for( unsigned int i=0; i<rRightHandSideVectors.size(); i++ )
    {
        this->InitializeSystemMatrices( LeftHandSideMatrix, rRightHandSideVectors[i], LocalSystem.CalculationFlags );
    }

    // Set Variables to Local system components
    LocalSystem.SetLeftHandSideMatrix(LeftHandSideMatrix);
    LocalSystem.SetRightHandSideVectors(rRightHandSideVectors);

    LocalSystem.SetRightHandSideVariables(rRHSVariables);

    // Calculate elemental system
    CalculateElementalSystem( LocalSystem, rCurrentProcessInfo );
}


//************************************************************************************
//************************************************************************************


void UpdatedLagrangianQuadrilateral::CalculateLeftHandSide( MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo )
{
    // Create local system components
    LocalSystemComponents LocalSystem;

    // Calculation flags
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX);

    VectorType RightHandSideVector = Vector();

    // Initialize sizes for the system components:
    this->InitializeSystemMatrices( rLeftHandSideMatrix, RightHandSideVector, LocalSystem.CalculationFlags );

    // Set Variables to Local system components
    LocalSystem.SetLeftHandSideMatrix(rLeftHandSideMatrix);
    LocalSystem.SetRightHandSideVector(RightHandSideVector);

    // Calculate elemental system
    CalculateElementalSystem( LocalSystem, rCurrentProcessInfo );
}
//************************************************************************************
//************************************************************************************


void UpdatedLagrangianQuadrilateral::CalculateLocalSystem( MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo )
{
    // Create local system components
    LocalSystemComponents LocalSystem;

    // Calculation flags
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX);
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR);

    // Initialize sizes for the system components:
    this->InitializeSystemMatrices( rLeftHandSideMatrix, rRightHandSideVector, LocalSystem.CalculationFlags );

    // Set Variables to Local system components
    LocalSystem.SetLeftHandSideMatrix(rLeftHandSideMatrix);
    LocalSystem.SetRightHandSideVector(rRightHandSideVector);

    // Calculate elemental system
    CalculateElementalSystem( LocalSystem, rCurrentProcessInfo );
}


//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::CalculateLocalSystem( std::vector< MatrixType >& rLeftHandSideMatrices,
        const std::vector< Variable< MatrixType > >& rLHSVariables,
        std::vector< VectorType >& rRightHandSideVectors,
        const std::vector< Variable< VectorType > >& rRHSVariables,
        ProcessInfo& rCurrentProcessInfo )
{
    //create local system components
    LocalSystemComponents LocalSystem;

    //calculation flags
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX_WITH_COMPONENTS);
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR_WITH_COMPONENTS);


    //Initialize sizes for the system components:
    if( rLHSVariables.size() != rLeftHandSideMatrices.size() )
        rLeftHandSideMatrices.resize(rLHSVariables.size());

    if( rRHSVariables.size() != rRightHandSideVectors.size() )
        rRightHandSideVectors.resize(rRHSVariables.size());

    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX);
    for( unsigned int i=0; i<rLeftHandSideMatrices.size(); i++ )
    {
        //Note: rRightHandSideVectors.size() > 0
        this->InitializeSystemMatrices( rLeftHandSideMatrices[i], rRightHandSideVectors[0], LocalSystem.CalculationFlags );
    }

    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_RHS_VECTOR);
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX,false);

    for( unsigned int i=0; i<rRightHandSideVectors.size(); i++ )
    {
        //Note: rLeftHandSideMatrices.size() > 0
        this->InitializeSystemMatrices( rLeftHandSideMatrices[0], rRightHandSideVectors[i], LocalSystem.CalculationFlags );
    }
    LocalSystem.CalculationFlags.Set(UpdatedLagrangianQuadrilateral::COMPUTE_LHS_MATRIX,true);


    // Set Variables to Local system components
    LocalSystem.SetLeftHandSideMatrices(rLeftHandSideMatrices);
    LocalSystem.SetRightHandSideVectors(rRightHandSideVectors);

    LocalSystem.SetLeftHandSideVariables(rLHSVariables);
    LocalSystem.SetRightHandSideVariables(rRHSVariables);

    // Calculate elemental system
    CalculateElementalSystem( LocalSystem, rCurrentProcessInfo );

}

////************************************************************************************
////************************************************************************************

void UpdatedLagrangianQuadrilateral::InitializeSolutionStep( ProcessInfo& rCurrentProcessInfo )
{
    /* NOTE:
    In the InitializeSolutionStep of each time step the nodal initial conditions are evaluated.
    This function is called by the base scheme class.*/

    GeometryType& r_geometry = GetGeometry();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    const unsigned int number_of_nodes = r_geometry.PointsNumber();
    const bool is_explicit = (rCurrentProcessInfo.Has(IS_EXPLICIT))
        ? rCurrentProcessInfo.GetValue(IS_EXPLICIT)
        : false;

    mFinalizedStep = false;

    // Calculating shape functions
    Vector N;
    this->MPMShapeFunctionPointValues(N, mMP.xg);

    array_1d<double,3> aux_MP_velocity = ZeroVector(3);
    array_1d<double,3> aux_MP_acceleration = ZeroVector(3);
    array_1d<double,3> nodal_momentum = ZeroVector(3);
    array_1d<double,3> nodal_inertia  = ZeroVector(3);


    if (!is_explicit)
    {
        for (unsigned int j = 0; j < number_of_nodes; j++)
        {
            // These are the values of nodal velocity and nodal acceleration evaluated in the initialize solution step
            array_1d<double, 3 > nodal_acceleration = ZeroVector(3);
            if (r_geometry[j].SolutionStepsDataHas(ACCELERATION))
                nodal_acceleration = r_geometry[j].FastGetSolutionStepValue(ACCELERATION, 1);

            array_1d<double, 3 > nodal_velocity = ZeroVector(3);
            if (r_geometry[j].SolutionStepsDataHas(VELOCITY))
                nodal_velocity = r_geometry[j].FastGetSolutionStepValue(VELOCITY, 1);

            for (unsigned int k = 0; k < dimension; k++)
            {
                aux_MP_velocity[k] += N[j] * nodal_velocity[k];
                aux_MP_acceleration[k] += N[j] * nodal_acceleration[k];
            }
        }
    }

    // Here MP contribution in terms of momentum, inertia and mass are added
    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        for (unsigned int j = 0; j < dimension; j++)
        {
            nodal_momentum[j] = N[i] * (mMP.velocity[j] - aux_MP_velocity[j]) * mMP.mass;
            nodal_inertia[j]  = N[i] * (mMP.acceleration[j] - aux_MP_acceleration[j]) * mMP.mass;
        }

        // Add in the predictor velocity increment for central difference explicit
        // This is the 'previous grid acceleration', which is actually
        // be the initial particle acceleration mapped to the grid.
        if (rCurrentProcessInfo.Has(IS_EXPLICIT_CENTRAL_DIFFERENCE)) {
            if (rCurrentProcessInfo.GetValue(IS_EXPLICIT_CENTRAL_DIFFERENCE)) {
                const double& delta_time = rCurrentProcessInfo[DELTA_TIME];
                for (unsigned int j = 0; j < dimension; j++) {
                    nodal_momentum[j] += 0.5 * delta_time * (N[i] * mMP.acceleration[j]) * mMP.mass;
                }
            }
        }

        r_geometry[i].SetLock();
        r_geometry[i].FastGetSolutionStepValue(NODAL_MOMENTUM, 0) += nodal_momentum;
        r_geometry[i].FastGetSolutionStepValue(NODAL_INERTIA, 0)  += nodal_inertia;
        r_geometry[i].FastGetSolutionStepValue(NODAL_MASS, 0) += N[i] * mMP.mass;
        r_geometry[i].UnSetLock();
    }
}

////************************************************************************************
////************************************************************************************

void UpdatedLagrangianQuadrilateral::FinalizeSolutionStep(ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const bool is_explicit = (rCurrentProcessInfo.Has(IS_EXPLICIT))
    ? rCurrentProcessInfo.GetValue(IS_EXPLICIT)
    : false;
    KRATOS_ERROR_IF(is_explicit)
        << "FinalizeSolutionStep for explicit time integration is done in the scheme";

    // Create and initialize element variables:
    GeneralVariables Variables;
    this->InitializeGeneralVariables(Variables, rCurrentProcessInfo);

    // Create constitutive law parameters:
    ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);

    // Set constitutive law flags:
    Flags& ConstitutiveLawOptions = Values.GetOptions();

    ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN);
    ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);

    // Compute element kinematics B, F, DN_DX ...
    this->CalculateKinematics(Variables, rCurrentProcessInfo);

    // Set general variables to constitutivelaw parameters
    this->SetGeneralVariables(Variables, Values);

    // Call the constitutive law to update material variables
    mConstitutiveLawVector->FinalizeMaterialResponse(Values, Variables.StressMeasure);

    // Call the element internal variables update
    this->FinalizeStepVariables(Variables, rCurrentProcessInfo);

    mFinalizedStep = true;
        
    KRATOS_CATCH("")
}

////************************************************************************************

void UpdatedLagrangianQuadrilateral::FinalizeStepVariables( GeneralVariables & rVariables, const ProcessInfo& rCurrentProcessInfo)
{
    // Update internal (historical) variables
    mDeterminantF0         = rVariables.detF* rVariables.detF0;
    mDeformationGradientF0 = prod(rVariables.F, rVariables.F0);

    mMP.cauchy_stress_vector = rVariables.StressVector;
    mMP.almansi_strain_vector = rVariables.StrainVector;

    // Delta Plastic Strains
    mConstitutiveLawVector->GetValue(MP_DELTA_PLASTIC_STRAIN, mMP.delta_plastic_strain);
    mConstitutiveLawVector->GetValue(MP_DELTA_PLASTIC_VOLUMETRIC_STRAIN, mMP.delta_plastic_volumetric_strain);
    mConstitutiveLawVector->GetValue(MP_DELTA_PLASTIC_DEVIATORIC_STRAIN, mMP.delta_plastic_deviatoric_strain);

    // Total Plastic Strain
    mConstitutiveLawVector->GetValue(MP_EQUIVALENT_PLASTIC_STRAIN, mMP.equivalent_plastic_strain);
    mConstitutiveLawVector->GetValue(MP_ACCUMULATED_PLASTIC_VOLUMETRIC_STRAIN, mMP.accumulated_plastic_volumetric_strain);
    mConstitutiveLawVector->GetValue(MP_ACCUMULATED_PLASTIC_DEVIATORIC_STRAIN, mMP.accumulated_plastic_deviatoric_strain);

    const bool is_explicit = (rCurrentProcessInfo.Has(IS_EXPLICIT))
        ? rCurrentProcessInfo.GetValue(IS_EXPLICIT)
        : false;
    if (!is_explicit) this->UpdateGaussPoint(rVariables, rCurrentProcessInfo);
}

//************************************************************************************
//************************************************************************************
/**
 * The position of the Gauss points/Material points is updated
 */

void UpdatedLagrangianQuadrilateral::UpdateGaussPoint( GeneralVariables & rVariables, const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    GeometryType& r_geometry = GetGeometry();
    rVariables.CurrentDisp = CalculateCurrentDisp(rVariables.CurrentDisp, rCurrentProcessInfo);
    const unsigned int number_of_nodes = r_geometry.PointsNumber();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();

    array_1d<double,3> delta_xg = ZeroVector(3);
    array_1d<double,3> MP_acceleration = ZeroVector(3);
    array_1d<double,3> MP_velocity = ZeroVector(3);
    const double& delta_time = rCurrentProcessInfo[DELTA_TIME];

    rVariables.N = this->MPMShapeFunctionPointValues(rVariables.N, mMP.xg);

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        if (rVariables.N[i] > std::numeric_limits<double>::epsilon() )
        {
            auto r_geometry = GetGeometry();
            array_1d<double, 3 > nodal_acceleration = ZeroVector(3);
            if (r_geometry[i].SolutionStepsDataHas(ACCELERATION))
                nodal_acceleration = r_geometry[i].FastGetSolutionStepValue(ACCELERATION);

            for ( unsigned int j = 0; j < dimension; j++ )
            {

                delta_xg[j] += rVariables.N[i] * rVariables.CurrentDisp(i,j);
                MP_acceleration[j] += rVariables.N[i] * nodal_acceleration[j];

                /* NOTE: The following interpolation techniques have been tried:
                    MP_velocity[j]      += rVariables.N[i] * nodal_velocity[j];
                    MP_acceleration[j]  += nodal_inertia[j]/(rVariables.N[i] * MP_mass * MP_number);
                    MP_velocity[j]      += nodal_momentum[j]/(rVariables.N[i] * MP_mass * MP_number);
                    MP_velocity[j]      += delta_time * rVariables.N[i] * nodal_acceleration[j];
                */
            }
        }

    }

    /* NOTE:
    Another way to update the MP velocity (see paper Guilkey and Weiss, 2003).
    This assume newmark (or trapezoidal, since n.gamma=0.5) rule of integration*/
    mMP.velocity = mMP.velocity + 0.5 * delta_time * (MP_acceleration + mMP.acceleration);

    /* NOTE: The following interpolation techniques have been tried:
        MP_acceleration = 4/(delta_time * delta_time) * delta_xg - 4/delta_time * MP_PreviousVelocity;
        MP_velocity = 2.0/delta_time * delta_xg - MP_PreviousVelocity;
    */

    // Update the MP Position
    mMP.xg = mMP.xg + delta_xg ;

    // Update the MP Acceleration
    mMP.acceleration = MP_acceleration;

    // Update the MP total displacement
    mMP.displacement += delta_xg;

    KRATOS_CATCH( "" )
}

void UpdatedLagrangianQuadrilateral::InitializeMaterial()
{
    KRATOS_TRY
    GeneralVariables Variables;

    if ( GetProperties()[CONSTITUTIVE_LAW] != NULL )
    {
        mConstitutiveLawVector = GetProperties()[CONSTITUTIVE_LAW]->Clone();

        Variables.N = this->MPMShapeFunctionPointValues(Variables.N, mMP.xg);

        mConstitutiveLawVector->InitializeMaterial( GetProperties(), GetGeometry(),
                Variables.N );

        mMP.almansi_strain_vector = ZeroVector(mConstitutiveLawVector->GetStrainSize());
        mMP.cauchy_stress_vector = ZeroVector(mConstitutiveLawVector->GetStrainSize());
    }
    else
        KRATOS_ERROR << "A constitutive law needs to be specified for the element with ID: " << this->Id() << std::endl;

    KRATOS_CATCH( "" )
}


//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::ResetConstitutiveLaw()
{
    KRATOS_TRY
    GeneralVariables Variables;

    if ( GetProperties()[CONSTITUTIVE_LAW] != NULL )
    {
        mConstitutiveLawVector->ResetMaterial(
            GetProperties(), GetGeometry(), this->MPMShapeFunctionPointValues(Variables.N, mMP.xg) );
    }

    KRATOS_CATCH( "" )
}




//*************************COMPUTE CURRENT DISPLACEMENT*******************************
//************************************************************************************
/*
This function convert the computed nodal displacement into matrix of (number_of_nodes, dimension)
*/
Matrix& UpdatedLagrangianQuadrilateral::CalculateCurrentDisp(Matrix & rCurrentDisp, const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.PointsNumber();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();

    rCurrentDisp = ZeroMatrix( number_of_nodes, dimension);

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        const array_1d<double, 3 > & current_displacement  = r_geometry[i].FastGetSolutionStepValue(DISPLACEMENT);

        for ( unsigned int j = 0; j < dimension; j++ )
        {
            rCurrentDisp(i,j) = current_displacement[j];
        }
    }

    return rCurrentDisp;

    KRATOS_CATCH( "" )
}


//*************************COMPUTE ALMANSI STRAIN*************************************
//************************************************************************************
// Almansi Strain: E = 0.5 (I - U^(-2))
void UpdatedLagrangianQuadrilateral::CalculateAlmansiStrain(const Matrix& rF,
        Vector& rStrainVector )
{
    KRATOS_TRY

    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

    // Left Cauchy-Green Calculation
    Matrix left_cauchy_green = prod( rF, trans( rF ) );

    // Calculating the inverse of the jacobian
    Matrix inv_left_cauchy_green ( dimension, dimension );
    double det_b=0;
    MathUtils<double>::InvertMatrix( left_cauchy_green, inv_left_cauchy_green, det_b);

    if( dimension == 2 )
    {
        // Almansi Strain Calculation
        rStrainVector[0] = 0.5 * (  1.00 - inv_left_cauchy_green( 0, 0 ) );
        rStrainVector[1] = 0.5 * (  1.00 - inv_left_cauchy_green( 1, 1 ) );
        rStrainVector[2] = - inv_left_cauchy_green( 0, 1 ); // xy
    }
    else if( dimension == 3 )
    {
        // Almansi Strain Calculation
        if ( rStrainVector.size() != 6 ) rStrainVector.resize( 6, false );
        rStrainVector[0] = 0.5 * (  1.00 - inv_left_cauchy_green( 0, 0 ) );
        rStrainVector[1] = 0.5 * (  1.00 - inv_left_cauchy_green( 1, 1 ) );
        rStrainVector[2] = 0.5 * (  1.00 - inv_left_cauchy_green( 2, 2 ) );
        rStrainVector[3] = - inv_left_cauchy_green( 0, 1 ); // xy
        rStrainVector[4] = - inv_left_cauchy_green( 1, 2 ); // yz
        rStrainVector[5] = - inv_left_cauchy_green( 0, 2 ); // xz
    }
    else
    {
        KRATOS_ERROR << "Dimension given is wrong!" << std::endl;
    }

    KRATOS_CATCH( "" )
}
//*************************COMPUTE GREEN-LAGRANGE STRAIN*************************************
//************************************************************************************
// Green-Lagrange Strain: E = 0.5 * (U^2 - I) = 0.5 * (C - I)
void UpdatedLagrangianQuadrilateral::CalculateGreenLagrangeStrain(const Matrix& rF,
        Vector& rStrainVector )
{
    KRATOS_TRY

    const unsigned int dimension  = GetGeometry().WorkingSpaceDimension();

    // Right Cauchy-Green Calculation
    Matrix C ( dimension, dimension );
    noalias( C ) = prod( trans( rF ), rF );

    if( dimension == 2 )
    {
        //Green Lagrange Strain Calculation
        if ( rStrainVector.size() != 3 ) rStrainVector.resize( 3, false );
        rStrainVector[0] = 0.5 * ( C( 0, 0 ) - 1.00 );
        rStrainVector[1] = 0.5 * ( C( 1, 1 ) - 1.00 );
        rStrainVector[2] = C( 0, 1 ); // xy
    }
    else if( dimension == 3 )
    {
        //Green Lagrange Strain Calculation
        if ( rStrainVector.size() != 6 ) rStrainVector.resize( 6, false );
        rStrainVector[0] = 0.5 * ( C( 0, 0 ) - 1.00 );
        rStrainVector[1] = 0.5 * ( C( 1, 1 ) - 1.00 );
        rStrainVector[2] = 0.5 * ( C( 2, 2 ) - 1.00 );
        rStrainVector[3] = C( 0, 1 ); // xy
        rStrainVector[4] = C( 1, 2 ); // yz
        rStrainVector[5] = C( 0, 2 ); // xz
    }
    else
    {
        KRATOS_ERROR << "Dimension given is wrong!" << std::endl;
    }

    KRATOS_CATCH( "" )
}



//************************************************************************************
//************************************************************************************

double& UpdatedLagrangianQuadrilateral::CalculateIntegrationWeight(double& rIntegrationWeight)
{
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

    if( dimension == 2 )
        rIntegrationWeight *= GetProperties()[THICKNESS];

    return rIntegrationWeight;
}


//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::EquationIdVector( EquationIdVectorType& rResult, ProcessInfo& CurrentProcessInfo )
{
    GeometryType& r_geometry = GetGeometry();
    int number_of_nodes = r_geometry.size();
    int dimension = r_geometry.WorkingSpaceDimension();
    unsigned int matrix_size = number_of_nodes * dimension;

    if ( rResult.size() != matrix_size )
        rResult.resize( matrix_size, false );

    for ( int i = 0; i < number_of_nodes; i++ )
    {
        int index = i * dimension;
        rResult[index] = r_geometry[i].GetDof( DISPLACEMENT_X ).EquationId();
        rResult[index + 1] = r_geometry[i].GetDof( DISPLACEMENT_Y ).EquationId();

        if ( dimension == 3 )
            rResult[index + 2] = r_geometry[i].GetDof( DISPLACEMENT_Z ).EquationId();
    }

}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::GetDofList( DofsVectorType& rElementalDofList, ProcessInfo& CurrentProcessInfo )
{
    rElementalDofList.resize( 0 );

    GeometryType& r_geometry = GetGeometry();
    for ( unsigned int i = 0; i < r_geometry.size(); i++ )
    {
        rElementalDofList.push_back( r_geometry[i].pGetDof( DISPLACEMENT_X ) );
        rElementalDofList.push_back( r_geometry[i].pGetDof( DISPLACEMENT_Y ) );

        if ( r_geometry.WorkingSpaceDimension() == 3 )
        {
            rElementalDofList.push_back( r_geometry[i].pGetDof( DISPLACEMENT_Z ) );
        }
    }
}


//************************************************************************************
//*******************DAMPING MATRIX***************************************************

void UpdatedLagrangianQuadrilateral::CalculateDampingMatrix( MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY

    //0.-Initialize the DampingMatrix:
    const unsigned int number_of_nodes = GetGeometry().size();
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

    //resizing as needed the LHS
    unsigned int matrix_size = number_of_nodes * dimension;

    if ( rDampingMatrix.size1() != matrix_size )
        rDampingMatrix.resize( matrix_size, matrix_size, false );

    noalias( rDampingMatrix ) = ZeroMatrix(matrix_size, matrix_size);

    //1.-Calculate StiffnessMatrix:
    MatrixType StiffnessMatrix  = Matrix();
    this->CalculateLeftHandSide( StiffnessMatrix, rCurrentProcessInfo );

    //2.-Calculate MassMatrix:
    MatrixType MassMatrix  = Matrix();
    this->CalculateMassMatrix ( MassMatrix, rCurrentProcessInfo );

    //3.-Get Damping Coeffitients (RAYLEIGH_ALPHA, RAYLEIGH_BETA)
    double alpha = 0;
    if( GetProperties().Has(RAYLEIGH_ALPHA) )
    {
        alpha = GetProperties()[RAYLEIGH_ALPHA];
    }
    else if( rCurrentProcessInfo.Has(RAYLEIGH_ALPHA) )
    {
        alpha = rCurrentProcessInfo[RAYLEIGH_ALPHA];
    }

    double beta  = 0;
    if( GetProperties().Has(RAYLEIGH_BETA) )
    {
        beta = GetProperties()[RAYLEIGH_BETA];
    }
    else if( rCurrentProcessInfo.Has(RAYLEIGH_BETA) )
    {
        beta = rCurrentProcessInfo[RAYLEIGH_BETA];
    }

    //4.-Compose the Damping Matrix:
    //Rayleigh Damping Matrix: alpha*M + beta*K
    rDampingMatrix  = alpha * MassMatrix;
    rDampingMatrix += beta  * StiffnessMatrix;

    KRATOS_CATCH( "" )
}

void UpdatedLagrangianQuadrilateral::AddExplicitContribution(const VectorType& rRHSVector,
    const Variable<VectorType>& rRHSVariable,
    const Variable<array_1d<double, 3>>& rDestinationVariable,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    if (rRHSVariable == RESIDUAL_VECTOR &&
        rDestinationVariable == FORCE_RESIDUAL)
    {
        GeometryType& r_geometry = GetGeometry();
        const unsigned int dimension = r_geometry.WorkingSpaceDimension();
        const unsigned int number_of_nodes = r_geometry.PointsNumber();

        for (size_t i = 0; i < number_of_nodes; ++i) {
            size_t index = dimension * i;
            array_1d<double, 3>& r_force_residual = r_geometry[i].FastGetSolutionStepValue(FORCE_RESIDUAL);
            for (size_t j = 0; j < dimension; ++j) {
                r_force_residual[j] += rRHSVector[index + j];
            }
        }
    }

    KRATOS_CATCH("")
}



//************************************************************************************
//****************MASS MATRIX*********************************************************

void UpdatedLagrangianQuadrilateral::CalculateMassMatrix( MatrixType& rMassMatrix, ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY

    //I need to call the values of the shape function for the single element
    GeneralVariables Variables;
    this->InitializeGeneralVariables(Variables,rCurrentProcessInfo);

    //lumped
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    const unsigned int number_of_nodes = GetGeometry().PointsNumber();
    unsigned int matrix_size = dimension * number_of_nodes;

    if ( rMassMatrix.size1() != matrix_size )
        rMassMatrix.resize( matrix_size, matrix_size, false );

    rMassMatrix = ZeroMatrix(matrix_size, matrix_size);

    //LUMPED MATRIX
    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        double temp = Variables.N[i] * mMP.mass;

        for ( unsigned int j = 0; j < dimension; j++ )
        {
            unsigned int index = i * dimension + j;
            rMassMatrix( index, index ) = temp;
        }
    }

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************

// Function that return Jacobian matrix
Matrix& UpdatedLagrangianQuadrilateral::MPMJacobian( Matrix& rResult, const array_1d<double,3>& rPoint)
{
    KRATOS_TRY

    // Derivatives of shape functions
    Matrix shape_functions_gradients;
    shape_functions_gradients = this->MPMShapeFunctionsLocalGradients(
                                   shape_functions_gradients, rPoint);

    const GeometryType& r_geometry = GetGeometry();
    const unsigned int number_nodes = r_geometry.PointsNumber();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();

    if (dimension ==2)
    {
        rResult.resize( 2, 2, false );
        rResult = ZeroMatrix(2,2);

        for ( unsigned int i = 0; i < number_nodes; i++ )
        {
            rResult( 0, 0 ) += ( r_geometry.GetPoint( i ).X() *  shape_functions_gradients( i, 0 ) );
            rResult( 0, 1 ) += ( r_geometry.GetPoint( i ).X() *  shape_functions_gradients( i, 1 ) );
            rResult( 1, 0 ) += ( r_geometry.GetPoint( i ).Y() *  shape_functions_gradients( i, 0 ) );
            rResult( 1, 1 ) += ( r_geometry.GetPoint( i ).Y() *  shape_functions_gradients( i, 1 ) );
        }
    }
    else if(dimension ==3)
    {
        rResult.resize( 3, 3, false );
        rResult = ZeroMatrix(3,3);

        for ( unsigned int i = 0; i < number_nodes; i++ )
        {
            rResult( 0, 0 ) += ( r_geometry.GetPoint( i ).X() *  shape_functions_gradients( i, 0 ) );
            rResult( 0, 1 ) += ( r_geometry.GetPoint( i ).X() *  shape_functions_gradients( i, 1 ) );
            rResult( 0, 2 ) += ( r_geometry.GetPoint( i ).X() *  shape_functions_gradients( i, 2 ) );
            rResult( 1, 0 ) += ( r_geometry.GetPoint( i ).Y() *  shape_functions_gradients( i, 0 ) );
            rResult( 1, 1 ) += ( r_geometry.GetPoint( i ).Y() *  shape_functions_gradients( i, 1 ) );
            rResult( 1, 2 ) += ( r_geometry.GetPoint( i ).Y() *  shape_functions_gradients( i, 2 ) );
            rResult( 2, 0 ) += ( r_geometry.GetPoint( i ).Z() *  shape_functions_gradients( i, 0 ) );
            rResult( 2, 1 ) += ( r_geometry.GetPoint( i ).Z() *  shape_functions_gradients( i, 1 ) );
            rResult( 2, 2 ) += ( r_geometry.GetPoint( i ).Z() *  shape_functions_gradients( i, 2 ) );
        }
    }

    return rResult;

    KRATOS_CATCH( "" )
}
/**
   * Jacobian in given point and given a delta position. This method calculate jacobian
   * matrix in given point and a given delta position.
   *
   * @param rPoint point which jacobians has to
* be calculated in it.
*
* @return Matrix of double which is jacobian matrix \f$ J \f$ in given point and a given delta position.
*
* @see DeterminantOfJacobian
* @see InverseOfJacobian
 */
Matrix& UpdatedLagrangianQuadrilateral::MPMJacobianDelta( Matrix& rResult, const array_1d<double,3>& rPoint, const Matrix & rDeltaPosition )
{
    KRATOS_TRY

    Matrix shape_functions_gradients;

    shape_functions_gradients = this->MPMShapeFunctionsLocalGradients(
                                    shape_functions_gradients, rPoint );

    const GeometryType& r_geometry = GetGeometry();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();

    if (dimension ==2)
    {
        rResult.resize( 2, 2, false );
        rResult = ZeroMatrix(2,2);

        for ( unsigned int i = 0; i < r_geometry.size(); i++ )
        {
            rResult( 0, 0 ) += ( r_geometry.GetPoint( i ).X() + rDeltaPosition(i,0)) * ( shape_functions_gradients( i, 0 ) );
            rResult( 0, 1 ) += ( r_geometry.GetPoint( i ).X() + rDeltaPosition(i,0)) * ( shape_functions_gradients( i, 1 ) );
            rResult( 1, 0 ) += ( r_geometry.GetPoint( i ).Y() + rDeltaPosition(i,1)) * ( shape_functions_gradients( i, 0 ) );
            rResult( 1, 1 ) += ( r_geometry.GetPoint( i ).Y() + rDeltaPosition(i,1)) * ( shape_functions_gradients( i, 1 ) );
        }
    }
    else if(dimension ==3)
    {
        rResult.resize( 3, 3, false );
        rResult = ZeroMatrix(3,3);
        for ( unsigned int i = 0; i < r_geometry.size(); i++ )
        {
            rResult( 0, 0 ) += ( r_geometry.GetPoint( i ).X() + rDeltaPosition(i,0)) * ( shape_functions_gradients( i, 0 ) );
            rResult( 0, 1 ) += ( r_geometry.GetPoint( i ).X() + rDeltaPosition(i,0)) * ( shape_functions_gradients( i, 1 ) );
            rResult( 0, 2 ) += ( r_geometry.GetPoint( i ).X() + rDeltaPosition(i,0)) * ( shape_functions_gradients( i, 2 ) );
            rResult( 1, 0 ) += ( r_geometry.GetPoint( i ).Y() + rDeltaPosition(i,1)) * ( shape_functions_gradients( i, 0 ) );
            rResult( 1, 1 ) += ( r_geometry.GetPoint( i ).Y() + rDeltaPosition(i,1)) * ( shape_functions_gradients( i, 1 ) );
            rResult( 1, 2 ) += ( r_geometry.GetPoint( i ).Y() + rDeltaPosition(i,1)) * ( shape_functions_gradients( i, 2 ) );
            rResult( 2, 0 ) += ( r_geometry.GetPoint( i ).Z() + rDeltaPosition(i,2)) * ( shape_functions_gradients( i, 0 ) );
            rResult( 2, 1 ) += ( r_geometry.GetPoint( i ).Z() + rDeltaPosition(i,2)) * ( shape_functions_gradients( i, 1 ) );
            rResult( 2, 2 ) += ( r_geometry.GetPoint( i ).Z() + rDeltaPosition(i,2)) * ( shape_functions_gradients( i, 2 ) );
        }
    }

    return rResult;

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************

/**
   * Shape function values in given point. This method calculate the shape function
   * vector in given point.
   *
   * @param rPoint point which shape function values have to
* be calculated in it.
*
* @return Vector of double which is shape function vector \f$ N \f$ in given point.
*
 */
Vector& UpdatedLagrangianQuadrilateral::MPMShapeFunctionPointValues( Vector& rResult, const array_1d<double,3>& rPoint )
{
    KRATOS_TRY

    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    array_1d<double,3> rPointLocal = ZeroVector(3);
    rPointLocal = GetGeometry().PointLocalCoordinates(rPointLocal, rPoint);

    if (dimension == 2)
    {
        rResult.resize( 4, false );

        // Shape Functions (if the first node of the connettivity is the node at the bottom left)
        rResult[0] = 0.25 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) ;
        rResult[1] = 0.25 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) ;
        rResult[2] = 0.25 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) ;
        rResult[3] = 0.25 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) ;

        //Shape Function (if the first node of the connettivity is the node at the top left)
        //rResult[0] = 0.25 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) ;
        //rResult[1] = 0.25 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) ;
        //rResult[2] = 0.25 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) ;
        //rResult[3] = 0.25 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) ;

        //Shape Function (if the first node of the connettivity is the node at the top right)
        //rResult[0] = 0.25 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) ;
        //rResult[1] = 0.25 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) ;
        //rResult[2] = 0.25 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) ;
        //rResult[3] = 0.25 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) ;

        //Shape Function (if the first node of the connettivity is the node at the bottom right)
        //rResult[0] = 0.25 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) ;
        //rResult[1] = 0.25 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) ;
        //rResult[2] = 0.25 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) ;
        //rResult[3] = 0.25 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) ;

    }
    else if (dimension == 3)
    {
        rResult.resize(8, false);

        // Shape Functions (if the first node of the connettivity is the node at (-1,-1,-1))
        // NOTE: Implemented based on Carlos Felippa's Lecture on AFEM Chapter 11
        rResult[0] = 0.125 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) * (1 - rPointLocal[2]) ;
        rResult[1] = 0.125 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) * (1 - rPointLocal[2]) ;
        rResult[2] = 0.125 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) * (1 - rPointLocal[2]) ;
        rResult[3] = 0.125 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) * (1 - rPointLocal[2]) ;
        rResult[4] = 0.125 * (1 - rPointLocal[0]) * (1 - rPointLocal[1]) * (1 + rPointLocal[2]) ;
        rResult[5] = 0.125 * (1 + rPointLocal[0]) * (1 - rPointLocal[1]) * (1 + rPointLocal[2]) ;
        rResult[6] = 0.125 * (1 + rPointLocal[0]) * (1 + rPointLocal[1]) * (1 + rPointLocal[2]) ;
        rResult[7] = 0.125 * (1 - rPointLocal[0]) * (1 + rPointLocal[1]) * (1 + rPointLocal[2]) ;
    }

    return rResult;

    KRATOS_CATCH( "" )
}



// Function which return dN/de
Matrix& UpdatedLagrangianQuadrilateral::MPMShapeFunctionsLocalGradients( Matrix& rResult, const array_1d<double,3>& rPoint)
{
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    array_1d<double,3> rPointLocal = ZeroVector(3);

    // Evaluate local coordinates of the MP position inside the background grid
    rPointLocal = GetGeometry().PointLocalCoordinates(rPointLocal, rPoint);
    if (dimension == 2)
    {
        rResult = ZeroMatrix(4,2);

        // Gradient Shape Function (if the first node of the connettivity is the node at the bottom left)
        rResult( 0, 0 ) = -0.25 * (1 - rPointLocal[1]);
        rResult( 0, 1 ) = -0.25 * (1 - rPointLocal[0]);
        rResult( 1, 0 ) = 0.25 * (1 - rPointLocal[1]);
        rResult( 1, 1 ) = -0.25 * (1 + rPointLocal[0]);
        rResult( 2, 0 ) = 0.25 * (1 + rPointLocal[1]);
        rResult( 2, 1 ) = 0.25 * (1 + rPointLocal[0]);
        rResult( 3, 0 ) = -0.25 * (1 + rPointLocal[1]);
        rResult( 3, 1 ) = 0.25 * (1 - rPointLocal[0]);

        // Gradient Shape Function (if the first node of the connettivity is the node at the top left)
        //rResult( 0, 0 ) = -0.25 * (1 + rPointLocal[1]);
        //rResult( 0, 1 ) = 0.25 * (1 - rPointLocal[0]);
        //rResult( 1, 0 ) = -0.25 * (1 - rPointLocal[1]);
        //rResult( 1, 1 ) = -0.25 * (1 - rPointLocal[0]);
        //rResult( 2, 0 ) = 0.25 * (1 - rPointLocal[1]);
        //rResult( 2, 1 ) = -0.25 * (1 + rPointLocal[0]);
        //rResult( 3, 0 ) = +0.25 * (1 + rPointLocal[1]);
        //rResult( 3, 1 ) = +0.25 * (1 + rPointLocal[0]);

        // Gradient Shape Function (if the first node of the connettivity is the node at the top right)
        //rResult( 0, 0 ) = 0.25 * (1 + rPointLocal[1]);
        //rResult( 0, 1 ) = 0.25 * (1 + rPointLocal[0]);
        //rResult( 1, 0 ) = -0.25 * (1 + rPointLocal[1]);
        //rResult( 1, 1 ) = 0.25 * (1 - rPointLocal[0]);
        //rResult( 2, 0 ) = -0.25 * (1 - rPointLocal[1]);
        //rResult( 2, 1 ) = -0.25 * (1 - rPointLocal[0]);
        //rResult( 3, 0 ) = +0.25 * (1 - rPointLocal[1]);
        //rResult( 3, 1 ) = -0.25 * (1 + rPointLocal[0]);

        // Gradient Shape Function (if the first node of the connettivity is the node at the bottom right)
        //rResult( 0, 0 ) = 0.25 * (1 - rPointLocal[1]);
        //rResult( 0, 1 ) = - 0.25 * (1 + rPointLocal[0]);
        //rResult( 1, 0 ) = 0.25 * (1 + rPointLocal[1]);
        //rResult( 1, 1 ) = 0.25 * (1 + rPointLocal[0]);
        //rResult( 2, 0 ) = - 0.25 * (1 + rPointLocal[1]);
        //rResult( 2, 1 ) = 0.25 * (1 - rPointLocal[0]);
        //rResult( 3, 0 ) = -0.25 * (1 - rPointLocal[1]);
        //rResult( 3, 1 ) = -0.25 * (1 - rPointLocal[0]);
    }
    else if(dimension == 3)
    {
        rResult = ZeroMatrix(8,3);

        // Gradient Shape Function (if the first node of the connectivity is at (-1,-1,-1))
        // NOTE: Implemented based on Carlos Felippa's Lecture on AFEM Chapter 11
        rResult(0,0) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(0,1) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[0]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(0,2) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[0]);

        rResult(1,0) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(1,1) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[0]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(1,2) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[0]);

        rResult(2,0) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(2,1) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[0]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(2,2) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[0]);

        rResult(3,0) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(3,1) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[0]) * (1.0 + -1.0 * rPointLocal[2]);
        rResult(3,2) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[0]);

        rResult(4,0) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(4,1) = 0.125 * -1.0 * (1.0 + -1.0 * rPointLocal[0]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(4,2) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[0]);

        rResult(5,0) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(5,1) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[0]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(5,2) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[0]);

        rResult(6,0) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(6,1) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[0]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(6,2) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[0]);

        rResult(7,0) = 0.125 * -1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(7,1) = 0.125 *  1.0 * (1.0 + -1.0 * rPointLocal[0]) * (1.0 +  1.0 * rPointLocal[2]);
        rResult(7,2) = 0.125 *  1.0 * (1.0 +  1.0 * rPointLocal[1]) * (1.0 + -1.0 * rPointLocal[0]);
    }

    return rResult;
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::GetValuesVector( Vector& values, int Step )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    unsigned int matrix_size = number_of_nodes * dimension;

    if ( values.size() != matrix_size ) values.resize( matrix_size, false );

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        unsigned int index = i * dimension;
        values[index] = r_geometry[i].FastGetSolutionStepValue( DISPLACEMENT_X, Step );
        values[index + 1] = r_geometry[i].FastGetSolutionStepValue( DISPLACEMENT_Y, Step );

        if ( dimension == 3 )
            values[index + 2] = r_geometry[i].FastGetSolutionStepValue( DISPLACEMENT_Z, Step );
    }
}


//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::GetFirstDerivativesVector( Vector& values, int Step )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    unsigned int matrix_size = number_of_nodes * dimension;

    if ( values.size() != matrix_size ) values.resize( matrix_size, false );

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        unsigned int index = i * dimension;
        values[index] = r_geometry[i].FastGetSolutionStepValue( VELOCITY_X, Step );
        values[index + 1] = r_geometry[i].FastGetSolutionStepValue( VELOCITY_Y, Step );

        if ( dimension == 3 )
            values[index + 2] = r_geometry[i].GetSolutionStepValue( VELOCITY_Z, Step );
    }
}

//************************************************************************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::GetSecondDerivativesVector( Vector& values, int Step )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    unsigned int matrix_size = number_of_nodes * dimension;

    if ( values.size() != matrix_size ) values.resize( matrix_size, false );

    for ( unsigned int i = 0; i < number_of_nodes; i++ )
    {
        unsigned int index = i * dimension;
        values[index] = r_geometry[i].FastGetSolutionStepValue( ACCELERATION_X, Step );
        values[index + 1] = r_geometry[i].FastGetSolutionStepValue( ACCELERATION_Y, Step );

        if ( dimension == 3 )
            values[index + 2] = r_geometry[i].FastGetSolutionStepValue( ACCELERATION_Z, Step );
    }
}
//************************************************************************************
//************************************************************************************
void UpdatedLagrangianQuadrilateral::GetHistoricalVariables( GeneralVariables& rVariables )
{
    //Deformation Gradient F ( set to identity )
    unsigned int size =  rVariables.F.size1();
    rVariables.detF  = 1;
    rVariables.F     = IdentityMatrix(size);

    rVariables.detF0 = mDeterminantF0;
    rVariables.F0    = mDeformationGradientF0;
}

//*************************DECIMAL CORRECTION OF STRAINS******************************
//************************************************************************************

void UpdatedLagrangianQuadrilateral::DecimalCorrection(Vector& rVector)
{
    KRATOS_TRY

    for ( unsigned int i = 0; i < rVector.size(); i++ )
    {
        if( rVector[i]*rVector[i]<1e-24 )
        {
            rVector[i]=0;
        }
    }

    KRATOS_CATCH( "" )
}


///@}
///@name Access Get Values
///@{

void UpdatedLagrangianQuadrilateral::CalculateOnIntegrationPoints(const Variable<bool>& rVariable,
    std::vector<bool>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == CALCULATE_EXPLICIT_MP_STRESS) 
    {
        GeneralVariables Variables;
        this->InitializeGeneralVariables(Variables, rCurrentProcessInfo);
        this->CalculateExplicitStresses(rCurrentProcessInfo, Variables);
        this->FinalizeStepVariables(Variables, rCurrentProcessInfo);
        rValues[0] = true;
    }
    else if (rVariable == EXPLICIT_MAP_GRID_TO_MP) 
    {
        Vector N;
        this->MPMShapeFunctionPointValues(N, mMP.xg);
        MPMExplicitUtilities::UpdateGaussPointExplicit(rCurrentProcessInfo, *this, N);
        rValues[0] = true;
    }
    else if (rVariable == CALCULATE_MUSL_VELOCITY_FIELD)
    {
        Vector N;
        this->MPMShapeFunctionPointValues(N, mMP.xg);
        MPMExplicitUtilities::CalculateMUSLGridVelocity(*this, N);
        rValues[0] = true;
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in CalculateOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::CalculateOnIntegrationPoints(const Variable<int>& rVariable,
    std::vector<int>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == MP_MATERIAL_ID) {
        rValues[0] = GetProperties().Id();
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in CalculateOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::CalculateOnIntegrationPoints(const Variable<double>& rVariable,
    std::vector<double>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == MP_DENSITY) {
        rValues[0] = mMP.density;
    }
    else if (rVariable == MP_MASS) {
        rValues[0] = mMP.mass;
    }
    else if (rVariable == MP_VOLUME) {
        rValues[0] = mMP.volume;
    }
    else if (rVariable == MP_POTENTIAL_ENERGY) {
        rValues[0] = MPMEnergyCalculationUtility::CalculatePotentialEnergy(*this);
    }
    else if (rVariable == MP_KINETIC_ENERGY) {
        rValues[0] = MPMEnergyCalculationUtility::CalculateKineticEnergy(*this);
    }
    else if (rVariable == MP_STRAIN_ENERGY) {
        rValues[0] = MPMEnergyCalculationUtility::CalculateStrainEnergy(*this);
    }
    else if (rVariable == MP_TOTAL_ENERGY) {
        rValues[0] = MPMEnergyCalculationUtility::CalculateTotalEnergy(*this);
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in CalculateOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::CalculateOnIntegrationPoints(const Variable<array_1d<double, 3 > >& rVariable,
    std::vector<array_1d<double, 3 > >& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == MP_COORD || rVariable == MPC_COORD) {
        rValues[0] = mMP.xg;
    }
    else if (rVariable == MP_DISPLACEMENT) {
        rValues[0] = mMP.displacement;
    }
    else if (rVariable == MP_VELOCITY) {
        rValues[0] = mMP.velocity;
    }
    else if (rVariable == MP_ACCELERATION) {
        rValues[0] = mMP.acceleration;
    }
    else if (rVariable == MP_VOLUME_ACCELERATION) {
        rValues[0] = mMP.volume_acceleration;
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in CalculateOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::CalculateOnIntegrationPoints(const Variable<Vector>& rVariable,
    std::vector<Vector>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == MP_CAUCHY_STRESS_VECTOR) {
        rValues[0] = mMP.cauchy_stress_vector;
    }
    else if (rVariable == MP_ALMANSI_STRAIN_VECTOR) {
        rValues[0] = mMP.almansi_strain_vector;
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in CalculateOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

///@}
///@name Access Set Values
///@{

void UpdatedLagrangianQuadrilateral::SetValuesOnIntegrationPoints(const Variable<int>& rVariable,
    std::vector<int>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
}

void UpdatedLagrangianQuadrilateral::SetValuesOnIntegrationPoints(const Variable<double>& rVariable,
    std::vector<double>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_ERROR_IF(rValues.size() > 1)
        << "Only 1 value per integration point allowed! Passed values vector size: "
        << rValues.size() << std::endl;

    if (rVariable == MP_MASS) {
        mMP.mass = rValues[0];
    }
    else if (rVariable == MP_DENSITY) {
        mMP.density = rValues[0];
    }
    else if (rVariable == MP_VOLUME) {
        mMP.volume = rValues[0];
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in SetValuesOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::SetValuesOnIntegrationPoints(const Variable<array_1d<double, 3 > >& rVariable,
    std::vector<array_1d<double, 3 > > rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_ERROR_IF(rValues.size() > 1)
        << "Only 1 value per integration point allowed! Passed values vector size: "
        << rValues.size() << std::endl;

    if (rVariable == MP_COORD || rVariable == MPC_COORD) {
        mMP.xg = rValues[0];
    }
    else if (rVariable == MP_DISPLACEMENT) {
        mMP.displacement = rValues[0];
    }
    else if (rVariable == MP_VELOCITY) {
        mMP.velocity = rValues[0];
    }
    else if (rVariable == MP_ACCELERATION) {
        mMP.acceleration = rValues[0];
    }
    else if (rVariable == MP_VOLUME_ACCELERATION) {
        mMP.volume_acceleration = rValues[0];
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in SetValuesOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

void UpdatedLagrangianQuadrilateral::SetValuesOnIntegrationPoints(const Variable<Vector>& rVariable,
    std::vector<Vector>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_ERROR_IF(rValues.size() > 1)
        << "Only 1 value per integration point allowed! Passed values vector size: "
        << rValues.size() << std::endl;

    if (rVariable == MP_CAUCHY_STRESS_VECTOR) {
        mMP.cauchy_stress_vector = rValues[0];
    }
    else if (rVariable == MP_ALMANSI_STRAIN_VECTOR) {
        mMP.almansi_strain_vector = rValues[0];
    }
    else
    {
        KRATOS_ERROR << "Variable " << rVariable << " is called in SetValuesOnIntegrationPoints, but is not implemented." << std::endl;
    }
}

///@}

/**
 * This function provides the place to perform checks on the completeness of the input.
 * It is designed to be called only once (or anyway, not often) typically at the beginning
 * of the calculations, so to verify that nothing is missing from the input
 * or that no common error is found.
 * @param rCurrentProcessInfo
 */
int  UpdatedLagrangianQuadrilateral::Check( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY

    Element::Check(rCurrentProcessInfo);

    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();

    // Verify compatibility with the constitutive law
    ConstitutiveLaw::Features LawFeatures;

    this->GetProperties().GetValue( CONSTITUTIVE_LAW )->GetLawFeatures(LawFeatures);

    bool correct_strain_measure = false;
    for(unsigned int i=0; i<LawFeatures.mStrainMeasures.size(); i++)
    {
        if(LawFeatures.mStrainMeasures[i] == ConstitutiveLaw::StrainMeasure_Deformation_Gradient)
            correct_strain_measure = true;
    }

    KRATOS_ERROR_IF_NOT(correct_strain_measure) << "Constitutive law is not compatible with the element type: Large Displacements " << std::endl;

    // Verify that the variables are correctly initialized
    KRATOS_CHECK_VARIABLE_KEY(DISPLACEMENT)
    KRATOS_CHECK_VARIABLE_KEY(VELOCITY)
    KRATOS_CHECK_VARIABLE_KEY(ACCELERATION)
    KRATOS_CHECK_VARIABLE_KEY(DENSITY)

    // Verify that the dofs exist
    for ( IndexType i = 0; i < number_of_nodes; i++ ) {
        const NodeType &rnode = this->GetGeometry()[i];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISPLACEMENT,rnode)

        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_X, rnode)
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Y, rnode)
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Z, rnode)
    }

    // Verify that the constitutive law exists
    if( this->GetProperties().Has( CONSTITUTIVE_LAW ) == false)
    {
        KRATOS_ERROR << "Constitutive law not provided for property " << this->GetProperties().Id() << std::endl;
    }
    else
    {
        // Verify that the constitutive law has the correct dimension
        if ( dimension == 2 )
        {
            KRATOS_CHECK_VARIABLE_KEY(THICKNESS)
            KRATOS_ERROR_IF_NOT(this->GetProperties().Has( THICKNESS )) << "THICKNESS not provided for element " << this->Id() << std::endl;
        }
        else
        {
            KRATOS_ERROR_IF_NOT(this->GetProperties().GetValue( CONSTITUTIVE_LAW )->GetStrainSize() == 6) << "Wrong constitutive law used. This is a 3D element! expected strain size is 6 (el id = ) " << this->Id() << std::endl;
        }

        // Check constitutive law
        this->GetProperties().GetValue( CONSTITUTIVE_LAW )->Check( this->GetProperties(), r_geometry, rCurrentProcessInfo );
    }

    return 0;

    KRATOS_CATCH( "" );
}


void UpdatedLagrangianQuadrilateral::save( Serializer& rSerializer ) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, Element )

    rSerializer.save("ConstitutiveLawVector",mConstitutiveLawVector);
    rSerializer.save("DeformationGradientF0",mDeformationGradientF0);
    rSerializer.save("DeterminantF0",mDeterminantF0);
}

void UpdatedLagrangianQuadrilateral::load( Serializer& rSerializer )
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, Element )
    rSerializer.load("ConstitutiveLawVector",mConstitutiveLawVector);
    rSerializer.load("DeformationGradientF0",mDeformationGradientF0);
    rSerializer.load("DeterminantF0",mDeterminantF0);
}


} // Namespace Kratos

