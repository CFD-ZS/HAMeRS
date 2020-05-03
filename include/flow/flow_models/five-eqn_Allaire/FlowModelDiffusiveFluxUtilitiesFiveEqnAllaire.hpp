#ifndef FLOW_MODEL_DIFFUSIVE_FLUX_UTILITIES_FIVE_EQN_ALLAIRE_HPP
#define FLOW_MODEL_DIFFUSIVE_FLUX_UTILITIES_FIVE_EQN_ALLAIRE_HPP

#include "flow/flow_models/FlowModelDiffusiveFluxUtilities.hpp"
#include "util/mixing_rules/equations_of_bulk_viscosity/EquationOfBulkViscosityMixingRulesManager.hpp"
#include "util/mixing_rules/equations_of_shear_viscosity/EquationOfShearViscosityMixingRulesManager.hpp"

class FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire: public FlowModelDiffusiveFluxUtilities
{
    public:
        FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire(
            const std::string& object_name,
            const tbox::Dimension& dim,
            const boost::shared_ptr<geom::CartesianGridGeometry>& grid_geometry,
            const int& num_species,
            const boost::shared_ptr<EquationOfShearViscosityMixingRules> equation_of_shear_viscosity_mixing_rules,
            const boost::shared_ptr<EquationOfBulkViscosityMixingRules> equation_of_bulk_viscosity_mixing_rules);
        
        ~FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire() {}
        
        /*
         * Register the required variables for the computation of diffusive fluxes in the registered patch.
         */
        void
        registerDerivedVariablesForDiffusiveFluxes(
            const hier::IntVector& num_subghosts);
        
        /*
         * Allocate memory for cell data of different registered derived variables in the registered patch for this class.
         */
        void allocateMemoryForDerivedCellData();
        
        /*
         * Clear cell data of different derived variables in the registered patch for this class.
         */
        void clearCellData();
        
        /*
         * Compute cell data of different registered derived variables for this class.
         */
        void computeDerivedCellData();
        
        /*
         * Get the variables for the derivatives in the diffusive fluxes.
         */
        void
        getCellDataOfDiffusiveFluxVariablesForDerivative(
            std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& derivative_var_data,
            std::vector<std::vector<int> >& derivative_var_component_idx,
            const DIRECTION::TYPE& flux_direction,
            const DIRECTION::TYPE& derivative_direction);
        
        /*
         * Get the diffusivities in the diffusive flux.
         */
        void
        getCellDataOfDiffusiveFluxDiffusivities(
            std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& diffusivities_data,
            std::vector<std::vector<int> >& diffusivities_component_idx,
            const DIRECTION::TYPE& flux_direction,
            const DIRECTION::TYPE& derivative_direction);
        
    private:
        /*
         * Set the number of sub-ghost cells of a variable.
         * This function can be called recursively if the variables are computed recursively.
         */
        void
        setNumberOfSubGhosts(
            const hier::IntVector& num_subghosts,
            const std::string& variable_name,
            const std::string& parent_variable_name);
        
        /*
         * Set the ghost boxes of derived cell variables.
         */
        void
        setDerivedCellVariableGhostBoxes();
        
        /*
         * Compute the cell data of shear viscosity in the registered patch.
         */
        void computeCellDataOfShearViscosity();
        
        /*
         * Compute the cell data of bulk viscosity in the registered patch.
         */
        void computeCellDataOfBulkViscosity();
        
        /*
         * Compute the cell data of diffusivities in the registered patch.
         */
        void
        computeCellDataOfDiffusivities();
        
        /*
         * Number of sub-ghost cells of derived cell data for this class.
         */
        hier::IntVector d_num_subghosts_shear_viscosity;
        hier::IntVector d_num_subghosts_bulk_viscosity;
        
        /*
         * Boxes with sub-ghost cells of derived cell data for this class.
         */
        hier::Box d_subghost_box_shear_viscosity;
        hier::Box d_subghost_box_bulk_viscosity;
        
        /*
         * Dimensions of boxes with sub-ghost cells of derived cell data for this class.
         */
        hier::IntVector d_subghostcell_dims_shear_viscosity;
        hier::IntVector d_subghostcell_dims_bulk_viscosity;
        
        /*
         * boost::shared_ptr to derived cell data for this class.
         */
        boost::shared_ptr<pdat::CellData<double> > d_data_shear_viscosity;
        boost::shared_ptr<pdat::CellData<double> > d_data_bulk_viscosity;
        
        /*
         * Whether derived cell data for this class is computed.
         */
        bool d_cell_data_shear_viscosity_computed;
        bool d_cell_data_bulk_viscosity_computed;
        
        /*
         * boost::shared_ptr to EquationOfShearViscosityMixingRules.
         */
        const boost::shared_ptr<EquationOfShearViscosityMixingRules>
            d_equation_of_shear_viscosity_mixing_rules;
        
        /*
         * boost::shared_ptr to EquationOfBulkViscosityMixingRules.
         */
        const boost::shared_ptr<EquationOfBulkViscosityMixingRules>
            d_equation_of_bulk_viscosity_mixing_rules;
        
};

#endif /* FLOW_MODEL_DIFFUSIVE_FLUX_UTILITIES_FIVE_EQN_ALLAIRE_HPP */
