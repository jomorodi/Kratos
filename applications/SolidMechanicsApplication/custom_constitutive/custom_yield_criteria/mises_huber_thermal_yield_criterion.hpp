//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:                July 2018 $
//   Revision:            $Revision:                  0.0 $
//
//

#if !defined(KRATOS_MISES_HUBER_THERMAL_YIELD_CRITERION_H_INCLUDED )
#define  KRATOS_MISES_HUBER_THERMAL_YIELD_CRITERION_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_constitutive/custom_yield_criteria/mises_huber_yield_criterion.hpp"

namespace Kratos
{
///@addtogroup ApplicationNameApplication
///@{

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

/// Short class definition.
/** Detail class definition.
*/
class KRATOS_API(SOLID_MECHANICS_APPLICATION) MisesHuberThermalYieldCriterion
	: public MisesHuberYieldCriterion
{
    public:
        ///@name Type Definitions
        ///@{

        /// Pointer definition of MisesHuberThermalYieldCriterion
        KRATOS_CLASS_POINTER_DEFINITION(MisesHuberThermalYieldCriterion);

        ///@}
        ///@name Life Cycle
        ///@{

        /// Default constructor.
        MisesHuberThermalYieldCriterion();

        /// Initialization constructor.
        MisesHuberThermalYieldCriterion(HardeningLawPointer pHardeningLaw);

        /// Copy constructor.
        MisesHuberThermalYieldCriterion(MisesHuberThermalYieldCriterion const& rOther);

        /// Assignment operator.
        MisesHuberThermalYieldCriterion& operator=(MisesHuberThermalYieldCriterion const& rOther);


        /// Destructor.
        ~MisesHuberThermalYieldCriterion() override;


        ///@}
        ///@name Operators
        ///@{

        /**
	 * Clone function (has to be implemented by any derived class)
	 * @return a pointer to a new instance of this yield criterion
	 */
        YieldCriterion::Pointer Clone() const override;

        ///@}
        ///@name Operations
        ///@{

        double& CalculatePlasticDissipation(double & rPlasticDissipation, const Parameters& rValues) override;

        double& CalculateDeltaPlasticDissipation(double & rDeltaPlasticDissipation, const Parameters& rValues) override;


        double& CalculateImplexPlasticDissipation(double & rPlasticDissipation, const Parameters& rValues) override;

        double& CalculateImplexDeltaPlasticDissipation(double & rDeltaPlasticDissipation, const Parameters& rValues) override;

        ///@}
        ///@name Access
        ///@{


        ///@}
        ///@name Inquiry
        ///@{


        ///@}
        ///@name Input and output
        ///@{

        // /// Turn back information as a string.
        // virtual std::string Info() const;

        // /// Print information about this object.
        // virtual void PrintInfo(std::ostream& rOStream) const;

        // /// Print object's data.
        // virtual void PrintData(std::ostream& rOStream) const;


        ///@}
        ///@name Friends
        ///@{


        ///@}

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


        ///@}
        ///@name Protected  Access
        ///@{


        ///@}
        ///@name Protected Inquiry
        ///@{


        ///@}
        ///@name Protected LifeCycle
        ///@{


        ///@}

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
	///@name Serialization
	///@{
	friend class Serializer;

	// A private default constructor necessary for serialization

	void save(Serializer& rSerializer) const override;

	void load(Serializer& rSerializer) override;

        ///@}
        ///@name Private Inquiry
        ///@{


        ///@}
        ///@name Un accessible methods
        ///@{

        ///@}

    }; // Class MisesHuberThermalYieldCriterion

    ///@}

    ///@name Type Definitions
    ///@{


    ///@}
    ///@name Input and output
    ///@{


    // /// input stream function
    // inline std::istream& operator >> (std::istream& rIStream,
    //                                   MisesHuberThermalYieldCriterion& rThis);

    // /// output stream function
    // inline std::ostream& operator << (std::ostream& rOStream,
    //                                   const MisesHuberThermalYieldCriterion& rThis)
    // {
    //     rThis.PrintInfo(rOStream);
    //     rOStream << std::endl;
    //     rThis.PrintData(rOStream);

    //     return rOStream;
    // }
    ///@}

    ///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_MISES_HUBER_THERMAL_YIELD_CRITERION_H_INCLUDED  defined
