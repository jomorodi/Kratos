//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Miguel Maso Sotomayor
//

#ifndef KRATOS_SHALLOW_WATER_RESIDUAL_BASED_BDF_SCHEME_H_INCLUDED
#define KRATOS_SHALLOW_WATER_RESIDUAL_BASED_BDF_SCHEME_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/checks.h"
#include "utilities/time_discretization.h"
#include "solving_strategies/schemes/residual_based_bdf_scheme.h"
#include "shallow_water_application_variables.h"

namespace Kratos
{
///@name Kratos Globals
///@{
///@}
///@name Type Definitions
///@{
///@}
///@name  Enum's
///@{
///@}
///@name  Functions
///@{
///@}
///@name Kratos Classes
///@{

/**
 * @class ShallowWaterResidualBasedBDFScheme
 * @ingroup KratosShallowWaterApplication
 * @brief BDF integration scheme (for dynamic problems)
 * @details The \f$n\f$ order Backward Differentiation Formula (BDF) method is a two step \f$n\f$ order accurate method.
 * This scheme is designed to solve a system of the type:
 * \f[
 *   \mathbf{M} \frac{du_{n0}}{dt} + \mathbf{K} u_{n0} = \mathbf{f}_{ext}
 * \f]
 * @author Miguel Maso Sotomayor
 */
template<class TSparseSpace,  class TDenseSpace>
class ShallowWaterResidualBasedBDFScheme
    : public ResidualBasedBDFScheme<TSparseSpace, TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{
    KRATOS_CLASS_POINTER_DEFINITION( ShallowWaterResidualBasedBDFScheme );

    typedef Scheme<TSparseSpace,TDenseSpace>                             BaseType;

    typedef typename BaseType::Pointer                            BaseTypePointer;

    typedef ResidualBasedBDFScheme<TSparseSpace,TDenseSpace>          BDFBaseType;

    typedef typename BDFBaseType::DofsArrayType                     DofsArrayType;

    typedef typename BDFBaseType::TSystemMatrixType             TSystemMatrixType;

    typedef typename BDFBaseType::TSystemVectorType             TSystemVectorType;

    typedef typename BDFBaseType::LocalSystemVectorType     LocalSystemVectorType;

    typedef typename BDFBaseType::LocalSystemMatrixType     LocalSystemMatrixType;

    typedef ModelPart::NodesContainerType                          NodesArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

    // Constructor
    explicit ShallowWaterResidualBasedBDFScheme(const std::size_t Order = 2)
        : BDFBaseType(Order)
    {}

    // Copy Constructor
    explicit ShallowWaterResidualBasedBDFScheme(ShallowWaterResidualBasedBDFScheme& rOther)
        : BDFBaseType(rOther)
    {}

    /**
     * Clone
     */
    BaseTypePointer Clone() override
    {
        return BaseTypePointer( new ShallowWaterResidualBasedBDFScheme(*this) );
    }

    // Destructor
    ~ShallowWaterResidualBasedBDFScheme() override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Performing the prediction of the solution
     * @details It predicts the solution for the current step
     * @param rModelPart The model of the problem to solve
     * @param rDofSet set of all primary variables
     * @param rA LHS matrix
     * @param rDx Incremental update of primary variables
     * @param rb RHS Vector
     */
    void Predict(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb
        ) override
    {
        KRATOS_TRY;

        const double delta_time = rModelPart.GetProcessInfo()[DELTA_TIME];

        const int num_nodes = static_cast<int>( rModelPart.Nodes().size() );
        const auto it_node_begin = rModelPart.Nodes().begin();

        const std::array<const Variable<double>*, 3> var_components = {&MOMENTUM_X, &MOMENTUM_Y, &HEIGHT};
        const std::array<const Variable<double>*, 3> accel_components = {&ACCELERATION_X, &ACCELERATION_Y, &VELOCITY_Z};

        #pragma omp parallel for
        for (int i = 0;  i < num_nodes; ++i) {
            auto it_node = it_node_begin + i;

            for (std::size_t j = 0; j < 3; ++j)
            {
                if (!it_node->IsFixed(*var_components[j])) {
                    double& un0 = it_node->FastGetSolutionStepValue(*var_components[j]);
                    double un1 = it_node->FastGetSolutionStepValue(*var_components[j], 1);
                    double dot_un1 = it_node->FastGetSolutionStepValue(*accel_components[j], 1);
                    un0 = un1 + delta_time * dot_un1;
                }
            }

            UpdateFirstDerivative(it_node);
        }

        KRATOS_CATCH( "" );
    }

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "ShallowWaterResidualBasedBDFScheme";
    }

    ///@}
    ///@name Friends
    ///@{

protected:

    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    /**
     * @brief Updating first time derivative
     * @param itNode the node interator
     */
    void UpdateFirstDerivative(NodesArrayType::iterator itNode) override
    {
        array_1d<double, 3>& dot_un0 = itNode->FastGetSolutionStepValue(ACCELERATION);
        double& dot_hn0 = itNode->FastGetSolutionStepValue(VELOCITY_Z);
        noalias(dot_un0) = BDFBaseType::mBDF[0] * itNode->FastGetSolutionStepValue(MOMENTUM);
        dot_hn0 = BDFBaseType::mBDF[0] * itNode->FastGetSolutionStepValue(HEIGHT);
        for (std::size_t i_order = 1; i_order < BDFBaseType::mOrder + 1; ++i_order)
        {
            noalias(dot_un0) += BDFBaseType::mBDF[i_order] * itNode->FastGetSolutionStepValue(MOMENTUM, i_order);
            dot_hn0 += BDFBaseType::mBDF[i_order] * itNode->FastGetSolutionStepValue(HEIGHT, i_order);
        }
    }

    /**
     * @brief Updating second time derivative
     * @param itNode the node interator
     */
    void UpdateSecondDerivative(NodesArrayType::iterator itNode) override {}

    /**
     * @brief It adds the dynamic LHS contribution of the elements
     * @param rLHS_Contribution The dynamic contribution for the LHS
     * @param rD The damping matrix
     * @param rM The mass matrix
     * @param rCurrentProcessInfo The current process info instance
     */
    void AddDynamicsToLHS(
        LocalSystemMatrixType& rLHS_Contribution,
        LocalSystemMatrixType& rD,
        LocalSystemMatrixType& rM,
        const ProcessInfo& rCurrentProcessInfo
        ) override
    {
        // Adding mass contribution to the dynamic stiffness
        if (rM.size1() != 0) { // if M matrix declared
            noalias(rLHS_Contribution) += rM * BDFBaseType::mBDF[0];
        }
    }

    /**
     * @brief It adds the dynamic RHS contribution of the elements
     * @param rElement The element to compute
     * @param RHS_Contribution The dynamic contribution for the RHS
     * @param D The damping matrix
     * @param M The mass matrix
     * @param rCurrentProcessInfo The current process info instance
     */
    void AddDynamicsToRHS(
        Element& rElement,
        LocalSystemVectorType& rRHS_Contribution,
        LocalSystemMatrixType& rD,
        LocalSystemMatrixType& rM,
        const ProcessInfo& rCurrentProcessInfo
        ) override
    {
        const std::size_t this_thread = OpenMPUtils::ThisThread();

        // Adding inertia contribution
        if (rM.size1() != 0) {
            rElement.GetFirstDerivativesVector(BDFBaseType::mVector.dotun0[this_thread], 0);
            noalias(rRHS_Contribution) -= prod(rM, BDFBaseType::mVector.dotun0[this_thread]);
        }
    }

    /**
     * @brief It adds the dynamic RHS contribution of the condition
     * @param rCondition The condition to compute
     * @param RHS_Contribution The dynamic contribution for the RHS
     * @param D The damping matrix
     * @param M The mass matrix
     * @param rCurrentProcessInfo The current process info instance
     */
    void AddDynamicsToRHS(
        Condition& rCondition,
        LocalSystemVectorType& rRHS_Contribution,
        LocalSystemMatrixType& rD,
        LocalSystemMatrixType& rM,
        const ProcessInfo& rCurrentProcessInfo
        ) override
    {
        const std::size_t this_thread = OpenMPUtils::ThisThread();

        // Adding inertia contribution
        if (rM.size1() != 0) {
            rCondition.GetFirstDerivativesVector(BDFBaseType::mVector.dotun0[this_thread], 0);
            noalias(rRHS_Contribution) -= prod(rM, BDFBaseType::mVector.dotun0[this_thread]);
        }
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{
    ///@{

private:

    ///@name Static Member Variables
    ///@{
    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{
    ///@}

    ///@}
    ///@name Serialization
    ///@{

    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class ShallowWaterResidualBasedBDFScheme

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

} // Namespace Kratos

#endif // KRATOS_SHALLOW_WATER_RESIDUAL_BASED_BDF_SCHEME_H_INCLUDED defined
