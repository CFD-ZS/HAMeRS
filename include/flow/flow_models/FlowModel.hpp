#ifndef FLOW_MODEL_HPP
#define FLOW_MODEL_HPP

#include "HAMeRS_config.hpp"

#include "algs/integrator/RungeKuttaLevelIntegrator.hpp"
#include "extn/visit_data_writer/ExtendedVisItDataWriter.hpp"
#include "flow/flow_models/FlowModelBasicUtilities.hpp"
#include "flow/flow_models/FlowModelBoundaryUtilities.hpp"
#include "flow/flow_models/FlowModelDiffusiveFluxUtilities.hpp"
#include "flow/flow_models/FlowModelRiemannSolver.hpp"
#include "flow/flow_models/FlowModelStatisticsUtilities.hpp"
#include "util/Directions.hpp"
#include "util/mixing_rules/equations_of_state/EquationOfStateMixingRulesManager.hpp"

#include "SAMRAI/appu/VisDerivedDataStrategy.h"
// #include "SAMRAI/appu/VisItDataWriter.h"
#include "SAMRAI/geom/CartesianGridGeometry.h"
#include "SAMRAI/geom/CartesianPatchGeometry.h"
#include "SAMRAI/pdat/CellData.h"
#include "SAMRAI/pdat/CellVariable.h"
#include "SAMRAI/pdat/SideData.h"

#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include <string>
#include <unordered_map>
#include <vector>

using namespace SAMRAI;

namespace EQN_FORM
{
    enum TYPE { CONSERVATIVE,
                ADVECTIVE };
}

namespace VAR
{
    enum TYPE { CONSERVATIVE,
                PRIMITIVE };
}

class FlowModelRiemannSolver;
class FlowModelBasicUtilities;
class FlowModelDiffusiveFluxUtilities;
class FlowModelStatisticsUtilities;

/*
 * The class should at least be able to provide the following cell data of the single-species/mixture:
 * DENSITY, MOMENTUM, TOTAL_ENERGY (per unit volume), VELOCITY, INTERNAL_ENERGY (per unit mass), PRESSURE,
 * SOUND_SPEED, PRIMITIVE_VARIABLES, CONVECTIVE_FLUX_X, CONVECTIVE_FLUX_Y, CONVECTIVE_FLUX_Z,
 * MAX_WAVE_SPEED_X, MAX_WAVE_SPEED_Y, MAX_WAVE_SPEED_Z.
 */
class FlowModel:
    public appu::VisDerivedDataStrategy,
    public boost::enable_shared_from_this<FlowModel> 
{
    public:
        FlowModel(
            const std::string& object_name,
            const tbox::Dimension& dim,
            const boost::shared_ptr<geom::CartesianGridGeometry>& grid_geometry,
            const int& num_species,
            const int& num_eqn,
            const boost::shared_ptr<tbox::Database>& flow_model_db):
                d_object_name(object_name),
                d_dim(dim),
                d_grid_geometry(grid_geometry),
                d_num_species(num_species),
                d_num_eqn(num_eqn),
                d_num_ghosts(-hier::IntVector::getOne(d_dim)),
                d_patch(nullptr),
                d_interior_box(hier::Box::getEmptyBox(dim)),
                d_ghost_box(hier::Box::getEmptyBox(dim)),
                d_interior_dims(hier::IntVector::getZero(d_dim)),
                d_ghostcell_dims(hier::IntVector::getZero(d_dim)),
                d_subdomain_box(hier::Box::getEmptyBox(dim)),
                d_derived_cell_data_computed(false)
        {
            NULL_USE(flow_model_db);
        }
        
        virtual ~FlowModel() {}
        
        /*
         * Get the total number of species.
         */
        int getNumberOfSpecies() const
        {
            return d_num_species;
        }
        
        /*
         * Get the total number of equations.
         */
        int getNumberOfEquations() const
        {
            return d_num_eqn;
        }
        
        /*
         * Return the form of each equation.
         */
        const std::vector<EQN_FORM::TYPE>& getEquationsForm() const
        {
            return d_eqn_form;
        }
        
        /*
         * Get the number of ghost cells of conservative variables.
         */
        const hier::IntVector&
        getNumberOfGhostCells() const
        {
            // Check whether a patch is already registered.
            if (!d_patch)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModel::getNumberOfGhostCells()\n"
                    << "No patch is registered yet."
                    << std::endl);
            }
            
            return d_num_ghosts;
        }
        
        /*
         * Return the boost::shared_ptr to the equation of state mixing rules.
         */
        const boost::shared_ptr<EquationOfStateMixingRules>&
        getEquationOfStateMixingRules() const
        {
            return d_equation_of_state_mixing_rules;
        }
        
        /*
         * Return the boost::shared_ptr to the Riemann solver object.
         */
        const boost::shared_ptr<FlowModelRiemannSolver>&
        getFlowModelRiemannSolver() const
        {
            return d_flow_model_riemann_solver;
        }
        
        /*
         * Return the boost::shared_ptr to the basic utilities object.
         */
        const boost::shared_ptr<FlowModelBasicUtilities>&
        getFlowModelBasicUtilities() const
        {
            return d_flow_model_basic_utilities;
        }
        
        /*
         * Return the boost::shared_ptr to the diffusive flux utilities object.
         */
        const boost::shared_ptr<FlowModelDiffusiveFluxUtilities>&
        getFlowModelDiffusiveFluxUtilities() const
        {
            return d_flow_model_diffusive_flux_utilities;
        }
        
        /*
         * Return the boost::shared_ptr to the boundary utilities object.
         */
        const boost::shared_ptr<FlowModelBoundaryUtilities>&
        getFlowModelBoundaryUtilities() const
        {
            return d_flow_model_boundary_utilities;
        }
        
        /*
         * Return the boost::shared_ptr to the statistics utilities object.
         */
        const boost::shared_ptr<FlowModelStatisticsUtilities>&
        getFlowModelStatisticsUtilities() const
        {
            return d_flow_model_statistics_utilities;
        }
        
        /*
         * Print all characteristics of the flow model class.
         */
        virtual void printClassData(std::ostream& os) const = 0;
        
        /*
         * Put the characteristics of the flow model class into the restart database.
         */
        virtual void
        putToRestart(
            const boost::shared_ptr<tbox::Database>& restart_db) const = 0;
        
        /*
         * Register the conservative variables.
         */
        virtual void
        registerConservativeVariables(
            RungeKuttaLevelIntegrator* integrator,
            const hier::IntVector& num_ghosts,
            const hier::IntVector& num_ghosts_intermediate) = 0;
        
        /*
         * Get the names of conservative variables.
         */
        virtual std::vector<std::string> getNamesOfConservativeVariables(bool have_underscores = false) = 0;
        
        /*
         * Get the names of primitive variables.
         */
        virtual std::vector<std::string> getNamesOfPrimitiveVariables(bool have_underscores = false) = 0;
        
        /*
         * Get the variable types of conservative variables.
         */
        virtual std::vector<std::string> getVariableTypesOfConservativeVariables() = 0;
        
        /*
         * Get the variable types of primitive variables.
         */
        virtual std::vector<std::string> getVariableTypesOfPrimitiveVariables() = 0;
        
        /*
         * Get the conservative variables.
         */
        virtual std::vector<boost::shared_ptr<pdat::CellVariable<double> > >
        getConservativeVariables() = 0;
        
        /*
         * Register a patch with a data context.
         */
        virtual void
        registerPatchWithDataContext(
            const hier::Patch& patch,
            const boost::shared_ptr<hier::VariableContext>& data_context) = 0;
        
        /*
         * Register different derived variables in the registered patch. The derived variables to be registered
         * are given as entires in a map of the variable name to the number of sub-ghost cells required.
         * If the variable to be registered is one of the conservative variable, the corresponding entry
         * in the map is ignored.
         */
        virtual void
        registerDerivedVariables(
            const std::unordered_map<std::string, hier::IntVector>& num_subghosts_of_data) = 0;
        
        /*
         * Unregister the registered patch. The registered data context and the cell data of all derived variables in
         * the patch are cleared.
         */
        virtual void unregisterPatch() = 0;
        
        /*
         * Check whether a patch is registered or not.
         */
        bool hasRegisteredPatch() const;
        
        /*
         * Get registered patch.
         */
        const hier::Patch& getRegisteredPatch() const;
        
        /*
         * Get sub-domain box.
         */
        const hier::Box& getSubdomainBox() const;
        
        /*
         * Set sub-domain box.
         */
        void setSubdomainBox(const hier::Box& subdomain_box);
        
        /*
         * Allocate memory for cell data of different registered derived variables.
         */
        virtual void allocateMemoryForDerivedCellData() = 0;
        
        /*
         * Compute the cell data of different registered derived variables with the registered data context.
         */
        virtual void computeDerivedCellData() = 0;
        
        /*
         * Get the cell data of one cell variable in the registered patch.
         */
        virtual boost::shared_ptr<pdat::CellData<double> >
        getCellData(const std::string& variable_key) = 0;
        
        /*
         * Get the cell data of different cell variables in the registered patch.
         */
        virtual std::vector<boost::shared_ptr<pdat::CellData<double> > >
        getCellData(const std::vector<std::string>& variable_keys) = 0;
        
        /*
         * Get the cell data of species cell variables in the registered patch.
         */
        virtual std::vector<boost::shared_ptr<pdat::CellData<double> > >
        getSpeciesCellData(const std::string& variable_key) = 0;
        
        /*
         * Fill the cell data of conservative variables in the interior box with value zero.
         */
        virtual void
        fillCellDataOfConservativeVariablesWithZero() = 0;
        
        /*
         * Update the cell data of conservative variables in the interior box after time advancement.
         */
        virtual void
        updateCellDataOfConservativeVariables() = 0;
        
        /*
         * Get the cell data of the conservative variables in the registered patch.
         */
        virtual std::vector<boost::shared_ptr<pdat::CellData<double> > >
        getCellDataOfConservativeVariables() = 0;
        
        /*
         * Get the cell data of the primitive variables in the registered patch.
         */
        virtual std::vector<boost::shared_ptr<pdat::CellData<double> > >
        getCellDataOfPrimitiveVariables() = 0;
        
        /*
         * Set the plotting context.
         */
        void
        setPlotContext(
            const boost::shared_ptr<hier::VariableContext>& plot_context)
        {
            d_plot_context = plot_context;
        }
        
        /*
         * Setup the Riemann solver object.
         */
        void
        setupRiemannSolver();
        
        /*
         * Setup the basic utilties object.
         */
        void
        setupBasicUtilities();
        
        /*
         * Setup the diffusive flux utilties object.
         */
        void
        setupDiffusiveFluxUtilities();
        
        /*
         * Setup the statistics utilties object.
         */
        void
        setupStatisticsUtilities();
        
        /*
         * Compute derived plot quantities registered with the VisIt data writers from data that
         * is maintained on each patch in the hierarchy.
         */
        virtual bool
        packDerivedDataIntoDoubleBuffer(
            double* buffer,
            const hier::Patch& patch,
            const hier::Box& region,
            const std::string& variable_name,
            int depth_id,
            double simulation_time) const = 0;
        
        /*
         * Register the plotting quantities.
         */
#ifdef HAVE_HDF5
        virtual void
        registerPlotQuantities(
            const boost::shared_ptr<ExtendedVisItDataWriter>& visit_writer) = 0;
#endif
        
    protected:
        /*
         * Set the context for data on a patch.
         */
        void
        setDataContext(const boost::shared_ptr<hier::VariableContext>& context)
        {
           d_data_context = context;
        }
        
        /*
         * Return boost::shared_ptr to patch data context.
         */
        boost::shared_ptr<hier::VariableContext>
        getDataContext() const
        {
           return d_data_context;
        }
        
        /*
         * Reset the data context to be null.
         */
        void
        clearDataContext()
        {
           d_data_context.reset();
        }
        
        /*
         * The object name is used for error/warning reporting.
         */
        const std::string d_object_name;
        
        /*
         * Problem dimension.
         */
        const tbox::Dimension d_dim;
        
        /*
         * boost::shared_ptr to the grid geometry.
         */
        const boost::shared_ptr<geom::CartesianGridGeometry> d_grid_geometry;
        
        /*
         * A string variable to describe the equation of state used.
         */
        std::string d_equation_of_state_str;
        
        /*
         * Number of species.
         */
        const int d_num_species;
        
        /*
         * Number of equations.
         */
        const int d_num_eqn;
        
        /*
         * boost::shared_ptr to EquationOfStateMixingRules.
         */
        boost::shared_ptr<EquationOfStateMixingRules> d_equation_of_state_mixing_rules;
        
        /*
         * boost::shared_ptr to EquationOfStateMixingRulesManager.
         */
        boost::shared_ptr<EquationOfStateMixingRulesManager> d_equation_of_state_mixing_rules_manager;
        
        /*
         * Form of each equation.
         */
        std::vector<EQN_FORM::TYPE> d_eqn_form;
        
        /*
         * Number of ghost cells for registered variables.
         */
        hier::IntVector d_num_ghosts;
        
        /*
         * Pointer to registered patch.
         */
        const hier::Patch* d_patch;
        
        /*
         * boost::shared_ptr to patch data context.
         */
        boost::shared_ptr<hier::VariableContext> d_data_context;
        
        /*
         * Interior box and box with ghost cells.
         */
        hier::Box d_interior_box;
        hier::Box d_ghost_box;
        
        /*
         * Dimensions of interior box and box with ghost cells.
         */
        hier::IntVector d_interior_dims;
        hier::IntVector d_ghostcell_dims;
        
        /*
         * Sub-domain box to compute the derived cell data.
         */
        hier::Box d_subdomain_box;
        
        /*
         * Whether all derived cell data is computed in full domain or sub-domain.
         */
        bool d_derived_cell_data_computed;
        
        /*
         * boost::shared_ptr to the plotting context.
         */
        boost::shared_ptr<hier::VariableContext> d_plot_context;
        
        /*
         * boost::shared_ptr to the Riemann solver object for the flow model.
         */
        boost::shared_ptr<FlowModelRiemannSolver> d_flow_model_riemann_solver;
        
        /*
         * boost::shared_ptr to the basic utilities object for the flow model.
         */
        boost::shared_ptr<FlowModelBasicUtilities> d_flow_model_basic_utilities;
        
        /*
         * boost::shared_ptr to the diffusive flux utilities object for the flow model.
         */
        boost::shared_ptr<FlowModelDiffusiveFluxUtilities> d_flow_model_diffusive_flux_utilities;
        
        /*
         * boost::shared_ptr to the boundary utilities object for the flow model.
         */
        boost::shared_ptr<FlowModelBoundaryUtilities> d_flow_model_boundary_utilities;
        
        /*
         * boost::shared_ptr to the statistics utilities object for the flow model.
         */
        boost::shared_ptr<FlowModelStatisticsUtilities> d_flow_model_statistics_utilities;
        
};

#endif /* FLOW_MODEL_HPP */
